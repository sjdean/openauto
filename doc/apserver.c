#define LIBUSB_API_VERSION 0x01000102

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libusb-1.0/libusb.h>

const char* accessory_info[] =
{
    "Android",
    "Android Auto",
    "Android Auto",
    "1.0",
    "",
    "",
    NULL
};

libusb_device_handle *cleanup_devices[32];
int cleanup_devices_cnt;

uint8_t ep_in, ep_out;
uint16_t ep_in_size;
libusb_device_handle *active_device;

int server_sock;
int client_sock;

size_t usb_tx_bufsize = 16384;
size_t usb_rx_bufsize = 32; // in multiplies of input ep maxsize

int usb_rx_pending;

struct transfer_link
{
    struct libusb_transfer *transfer;
    size_t transferred;
    struct transfer_link *next;
};

struct transfer_link *rx_transfers;

void usb_tx_callback(struct libusb_transfer *recv)
{
    if (recv->user_data)
        libusb_dev_mem_free(recv->dev_handle, recv->buffer, usb_tx_bufsize);
    else
        free(recv->buffer);
}

void usb_rx_callback(struct libusb_transfer *recv)
{
    usb_rx_pending = 0;
    if (recv->status != LIBUSB_TRANSFER_COMPLETED) {
        if (recv->user_data)
            libusb_dev_mem_free(recv->dev_handle, recv->buffer, usb_tx_bufsize);
        else
            free(recv->buffer);
    }

    struct transfer_link *link = calloc(1, sizeof(struct transfer_link));
    link->transfer = recv;

    if (!rx_transfers) {
        rx_transfers = link;
    }
    else {
        struct transfer_link *last = rx_transfers;
        while (last->next)
            last = last->next;
        last->next = link;
    }
}

void cleanup_callback(struct libusb_transfer *recv)
{
    cleanup_devices[cleanup_devices_cnt++] = recv->dev_handle;
}

void modeswitch_callback(struct libusb_transfer *recv)
{
    if (recv->status != LIBUSB_TRANSFER_COMPLETED) {
        cleanup_devices[cleanup_devices_cnt++] = recv->dev_handle;
        return;
    }

    printf("accessory protocol: %d %d\n",
        recv->buffer[LIBUSB_CONTROL_SETUP_SIZE],
        recv->buffer[LIBUSB_CONTROL_SETUP_SIZE + 1]);

    if (recv->buffer[LIBUSB_CONTROL_SETUP_SIZE] >= 1) {
        int id = 0;
        for (const char **str = &accessory_info[0]; *str != NULL; str++)
        {
            int len = strlen(*str) + 1;
            uint8_t *buf = malloc(LIBUSB_CONTROL_SETUP_SIZE + len);
            strcpy((char*)&buf[LIBUSB_CONTROL_SETUP_SIZE], *str);
            libusb_fill_control_setup(buf, 0x40, 52, 0, id++, len);

            struct libusb_transfer *transfer = libusb_alloc_transfer(0);
            libusb_fill_control_transfer(transfer, recv->dev_handle, buf, NULL, NULL, 1000);
            transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER | LIBUSB_TRANSFER_FREE_TRANSFER;

            if (libusb_submit_transfer(transfer) != 0) {
                cleanup_devices[cleanup_devices_cnt++] = recv->dev_handle;
                return;
            }
        }

        {
            uint8_t *buf = malloc(LIBUSB_CONTROL_SETUP_SIZE);
            libusb_fill_control_setup(buf, 0x40, 53, 0, 0, 0);

            struct libusb_transfer *transfer = libusb_alloc_transfer(0);
            libusb_fill_control_transfer(transfer, recv->dev_handle, buf, cleanup_callback, NULL, 1000);
            transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER | LIBUSB_TRANSFER_FREE_TRANSFER;

            libusb_submit_transfer(transfer);
        }
    }
}

void exit_cleanup(void)
{
    if (active_device) {
        libusb_release_interface(active_device, 0);
        libusb_close(active_device);
    }

    if (server_sock > 0)
        close(server_sock);

    if (client_sock > 0)
        close(client_sock);

   libusb_exit(NULL);
}

void setup_sock_server(void)
{
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);

    fcntl(server_sock, F_SETFL, fcntl(server_sock, F_GETFL, 0) | O_NONBLOCK);

    struct sockaddr_un sa;
    sa.sun_family = AF_UNIX;
    strcpy(sa.sun_path, "./unixsock");
    unlink(sa.sun_path);

    if (bind(server_sock, (struct sockaddr *) &sa, sizeof(sa)))
        exit(4);

    if (listen(server_sock, 1))
        exit(5);

    chmod(sa.sun_path, 0666);

    printf("socket listening\n");
}

int usb_plug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    if (active_device)
        return 1;

    libusb_device_handle *handle;
    if (libusb_open(dev, &handle) != 0) {
        printf("failed to open device\n");
        return 0;
    }

    if (libusb_kernel_driver_active(handle, 0) != 0) {
        printf("ignoring, kernel driver active\n");
        libusb_close(handle);
        return 0;
    }

    struct libusb_device_descriptor desc;
    libusb_get_device_descriptor(dev, &desc);

    if (desc.idVendor == 0x18D1 && (desc.idProduct == 0x2D00 || desc.idProduct == 0x2D01))
    {
        printf("accessory connected\n");

        struct libusb_config_descriptor *config;
        if (libusb_get_active_config_descriptor(dev, &config) != 0) {
            printf("failed to get config descriptor\n");
            libusb_close(handle);
            return 0;
        }

        ep_in = 0;
        ep_out = 0;

        for (int i = 0; i < config->bNumInterfaces; i++) {
            if (config->interface[i].num_altsetting != 1)
                continue;

            if (config->interface[i].altsetting[0].bInterfaceNumber != 0)
                continue;

            if (config->interface[i].altsetting[0].bNumEndpoints != 2)
                continue;

            for (int j = 0; j < 2; j++) {
                uint8_t addr = config->interface[i].altsetting[0].endpoint[j].bEndpointAddress;
                if (addr & LIBUSB_ENDPOINT_IN) {
                    ep_in = addr;
                    ep_in_size = config->interface[i].altsetting[0].endpoint[j].wMaxPacketSize;
                }
                else
                    ep_out = addr;
            }
        }

        libusb_free_config_descriptor(config);

        if (ep_in && ep_out) {
            if (libusb_claim_interface(handle, 0) != 0) {
                printf("failed to claim interface\n");
                libusb_close(handle);
                return 0;
            }

            printf("starting\n");
            active_device = handle;
        } else {
            printf("proper endpoints not found\n");
            libusb_close(handle);
            return 0;
        }
    }
    else {
        printf("some device connected\n");

        uint8_t *buf = malloc(LIBUSB_CONTROL_SETUP_SIZE + 2);
        libusb_fill_control_setup(buf, 0xC0, 51, 0, 0, 2);

        struct libusb_transfer *transfer = libusb_alloc_transfer(0);
        libusb_fill_control_transfer(transfer, handle, buf, modeswitch_callback, NULL, 1000);
        transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER | LIBUSB_TRANSFER_FREE_TRANSFER;

        if (libusb_submit_transfer(transfer) != 0)
            libusb_close(handle);
    }

    return 0;
}

int main(void)
{
    if (libusb_init(NULL))
        return 1;

    atexit(exit_cleanup);

    if (libusb_pollfds_handle_timeouts(NULL) != 1)
        return 2;

    setup_sock_server();

    libusb_hotplug_register_callback(
        NULL,
        LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,
        LIBUSB_HOTPLUG_ENUMERATE,
        LIBUSB_HOTPLUG_MATCH_ANY,
        LIBUSB_HOTPLUG_MATCH_ANY,
        LIBUSB_HOTPLUG_MATCH_ANY, usb_plug_callback, NULL, NULL);

    while (1) {
        const struct libusb_pollfd **usb_fds = libusb_get_pollfds(NULL);

        const struct libusb_pollfd **it = usb_fds;
        int usb_fd_cnt = 0;
        while (*(it++) != NULL)
            usb_fd_cnt++;

        int fd_cnt = usb_fd_cnt + 1;

        struct pollfd all_fds[fd_cnt];

        for (int i = 0; i < usb_fd_cnt; i++) {
            all_fds[i].fd = usb_fds[i]->fd;
            all_fds[i].events = usb_fds[i]->events;
            all_fds[i].revents = 0;
        }

        if (client_sock > 0) {
            all_fds[usb_fd_cnt].fd = client_sock;
            all_fds[usb_fd_cnt].events = 0;
            if (active_device)
                all_fds[usb_fd_cnt].events |= POLLIN;
            if (rx_transfers)
                all_fds[usb_fd_cnt].events |= POLLOUT;
            all_fds[usb_fd_cnt].revents = 0;
        } else {
            all_fds[usb_fd_cnt].fd = server_sock;
            all_fds[usb_fd_cnt].events = POLLIN;
            all_fds[usb_fd_cnt].revents = 0;
        }

        libusb_free_pollfds(usb_fds);

        poll(all_fds, fd_cnt, -1);

        int usb_ev = 0;
        for (int i = 0; i < usb_fd_cnt; i++)
            usb_ev += all_fds[i].revents;

        if (usb_ev) {
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 0;

            if (libusb_handle_events_timeout_completed(NULL, &tv, NULL))
                return 3;

            for (int i = 0; i < cleanup_devices_cnt; i++)
                libusb_close(cleanup_devices[i]);
            cleanup_devices_cnt = 0;
        }

        if (client_sock > 0 && active_device && all_fds[usb_fd_cnt].revents & POLLIN) {
            void* special_free = (void*)1;
            uint8_t *buf = libusb_dev_mem_alloc(active_device, usb_tx_bufsize);
            if (!buf) {
                special_free = NULL;
                buf = malloc(usb_tx_bufsize);
            }

            int bytes = recv(client_sock, buf, usb_tx_bufsize, 0);
            if (bytes <= 0)
                exit(9);

            struct libusb_transfer *transfer = libusb_alloc_transfer(0);
            libusb_fill_bulk_transfer(transfer, active_device, ep_out, buf, bytes, usb_tx_callback, special_free, 0);
            transfer->flags = LIBUSB_TRANSFER_ADD_ZERO_PACKET | LIBUSB_TRANSFER_FREE_TRANSFER;

            if (libusb_submit_transfer(transfer) != 0)
                exit(7);
        }

        if (active_device && !usb_rx_pending) {
            void* special_free = (void*)1;
            uint8_t *buf = libusb_dev_mem_alloc(active_device, ep_in_size * usb_rx_bufsize);
            if (!buf) {
                special_free = NULL;
                buf = malloc(ep_in_size * usb_rx_bufsize);
            }

            struct libusb_transfer *transfer = libusb_alloc_transfer(0);
            libusb_fill_bulk_transfer(transfer, active_device, ep_in, buf, ep_in_size * usb_rx_bufsize, usb_rx_callback, special_free, 0);
            transfer->flags = 0;

            if (libusb_submit_transfer(transfer) != 0)
                exit(8);

            usb_rx_pending = 1;
        }

        if (client_sock > 0 && all_fds[usb_fd_cnt].revents & POLLOUT) {
            int bytes = send(client_sock, rx_transfers->transfer->buffer + rx_transfers->transferred, rx_transfers->transfer->actual_length - rx_transfers->transferred, 0);
            if (bytes < 0)
                exit(10);

            rx_transfers->transferred += bytes;
            if (rx_transfers->transferred == rx_transfers->transfer->actual_length) {
                if (rx_transfers->transfer->user_data)
                    libusb_dev_mem_free(rx_transfers->transfer->dev_handle, rx_transfers->transfer->buffer, ep_in_size * usb_rx_bufsize);
                else
                    free(rx_transfers->transfer->buffer);
                libusb_free_transfer(rx_transfers->transfer);
                struct transfer_link *next = rx_transfers->next;
                free(rx_transfers);
                rx_transfers = next;
            }
        }

        if (client_sock <= 0 && all_fds[usb_fd_cnt].revents & POLLIN) {
            client_sock = accept(server_sock, NULL, NULL);

            if (client_sock == -1)
                exit(6);

            fcntl(client_sock, F_SETFL, fcntl(client_sock, F_GETFL, 0) | O_NONBLOCK);
            printf("socket connected\n");
        }
    }

    return 0;
}