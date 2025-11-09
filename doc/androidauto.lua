-- this dissects aap over tcp 5277
-- usb is the same protocol pushed through bulk endpoints but I don't think wireshark does packet reassembly for these
-- to see all data you need to dump session keys from headunit executable, configure wireshark TLS dissector as usual
-- you also need .proto files dumped from binary, configure paths to these in wireshark protobuf dissector

-- caveat: we make hardcoded assumptions about service ids
-- this is probably fine for basic services, but for more we need to properly parse ServiceDiscoveryResponse and store mapping table somewhere

local aap_protocol = Proto("aap", "Android Auto")

local channel_id = ProtoField.uint8("aap.channel_id", "Channel ID", base.HEX)
local flags = ProtoField.uint8("aap.flags", "Flags", base.HEX)
local fraginfo = ProtoField.uint8("aap.flags.fraginfo", "Fragmentation", base.HEX,
	{ [0] = "FRAG_CONTINUATION",
	  [1] = "FRAG_FIRST",
	  [2] = "FRAG_LAST",
	  [3] = "FRAG_UNFRAGMENTED"}, 0x03)
local msgtype = ProtoField.uint8("aap.flags.type", "Type", base.HEX,
	{ [0] = "Channel specific",
	  [1] = "Control"}, 0x04)
local encflag = ProtoField.uint8("aap.flags.encrypted", "Encrypted", base.HEX,
	{ [0] = "No",
	  [1] = "Yes"}, 0x08)
local length = ProtoField.uint32("aap.len", "Length", base.DEC)
local verfield = ProtoField.uint32("aap.version", "Version", base.HEX)
local verfieldmaj = ProtoField.uint16("aap.version.major", "Major version", base.DEC)
local verfieldmin = ProtoField.uint16("aap.version.minor", "Minor version", base.DEC)

local packettypes =
{
	[1] = "MESSAGE_VERSION_REQUEST",
	[2] = "MESSAGE_VERSION_RESPONSE",
	[3] = "MESSAGE_ENCAPSULATED_SSL",
	[4] = "MESSAGE_AUTH_COMPLETE",
	[5] = "MESSAGE_SERVICE_DISCOVERY_REQUEST",
	[6] = "MESSAGE_SERVICE_DISCOVERY_RESPONSE",
	[7] = "MESSAGE_CHANNEL_OPEN_REQUEST",
	[8] = "MESSAGE_CHANNEL_OPEN_RESPONSE",
	[9] = "MESSAGE_CHANNEL_CLOSE_NOTIFICATION",
	[11] = "MESSAGE_PING_REQUEST",
	[12] = "MESSAGE_PING_RESPONSE",
	[13] = "MESSAGE_NAV_FOCUS_REQUEST",
	[14] = "MESSAGE_NAV_FOCUS_NOTIFICATION",
	[15] = "MESSAGE_BYEBYE_REQUEST",
	[16] = "MESSAGE_BYEBYE_RESPONSE",
	[17] = "MESSAGE_VOICE_SESSION_NOTIFICATION",
	[18] = "MESSAGE_AUDIO_FOCUS_REQUEST",
	[19] = "MESSAGE_AUDIO_FOCUS_NOTIFICATION",
	[20] = "MESSAGE_CAR_CONNECTED_DEVICES_REQUEST",
	[21] = "MESSAGE_CAR_CONNECTED_DEVICES_RESPONSE",
	[22] = "MESSAGE_USER_SWITCH_REQUEST",
	[23] = "MESSAGE_BATTERY_STATUS_NOTIFICATION",
	[24] = "MESSAGE_CALL_AVAILABILITY_STATUS",
	[25] = "MESSAGE_USER_SWITCH_RESPONSE",
	[26] = "MESSAGE_SERVICE_DISCOVERY_UPDATE",
	[255] = "MESSAGE_UNEXPECTED_MESSAGE",
	[65535] = "MESSAGE_FRAMING_ERROR"
}

local messagemapping =
{
	-- mapping: service id -> message id -> protobuf message type
	-- as mentioned before we shouldn't be hardcoding service ids and instead read these from ServiceDiscoveryResponse
	[0] = { -- control messages
		[1] = "VersionRequestOptions",
		[2] = "VersionResponseOptions",
		[4] = "AuthResponse",
		[5] = "ServiceDiscoveryRequest",
		[6] = "ServiceDiscoveryResponse",
		[7] = "ChannelOpenRequest",
		[8] = "ChannelOpenResponse",
		[9] = "ChannelCloseNotification",
		[11] = "PingRequest",
		[12] = "PingResponse",
		[13] = "NavFocusRequestNotification",
		[14] = "NavFocusNotification",
		[15] = "ByeByeRequest",
		[16] = "ByeByeResponse",
		[17] = "VoiceSessionNotification",
		[18] = "AudioFocusRequestNotification",
		[19] = "AudioFocusNotification",
		[20] = "CarConnectedDevicesRequest",
		[21] = "CarConnectedDevices",
		[22] = "UserSwitchRequest",
		[23] = "BatteryStatusNotification",
		[24] = "CallAvailabilityStatus",
		[25] = "UserSwitchResponse",
		[26] = "ServiceDiscoveryUpdate",
	},
	[1] = { -- sensor_source_service
		[32769] = "SensorRequest",
		[32770] = "SensorResponse",
		[32771] = "SensorBatch",
		[32772] = "SensorError",
	},
	[2] = { -- media_sink_service
		[32768] = "Setup",
		[32769] = "Start",
		[32770] = "Stop",
		[32771] = "Config",
		[32772] = "Ack",
		[32773] = "MicrophoneRequest",
		[32774] = "MicrophoneResponse",
		[32776] = "VideoFocusRequestNotification",
		[32777] = "UpdateUiConfigRequest",
		[32778] = "UpdateUiConfigReply",
		[32779] = "AudioUnderflowNotification",
	},
	[3] = { -- input_source_service
		[32769] = "InputReport",
		[32770] = "KeyBindingRequest",
		[32771] = "KeyBindingResponse",
		[32772] = "InputFeedback",
	}
	-- todo: add more
}

local controltype = ProtoField.uint16("aap.controlmsgtype", "Control message type", base.HEX, packettypes)
aap_protocol.fields = { channel_id, flags, fraginfo, msgtype, encflag, length,
	controltype, verfield, verfieldmaj, verfieldmin }

local tls_dissector = Dissector.get("tls")

local protobuf_dissector = Dissector.get("protobuf")

local dissect_payload = function(buffer, pinfo, tree)
	local service = messagemapping[tonumber(pinfo.private["channel_id"])]
	if service == nil then
		return
	end
	local type = service[buffer(0, 2):uint()]

	if type ~= nil then
		local offset = 2
		if type == "VersionRequestOptions" then
			offset = offset + 4
		elseif type == "VersionResponseOptions" then
			offset = offset + 6
		end
		if offset == buffer:len() then
			return
		end
		pinfo.private["pb_msg_type"] = "message," .. type
		protobuf_dissector:call(buffer(offset):tvb(), pinfo, tree)
	end
end

local dissect_packet = function(buffer, pinfo, tree)
	if buffer:len() < 4 then
		pinfo.desegment_len = DESEGMENT_ONE_MORE_SEGMENT
		return -1
	end

	local pktbeg = 0
	local pktlen = 0

	local enc = bit.band(buffer(1, 1):uint(), 0x08) == 0x08

	if bit.band(buffer(1, 1):uint(), 0x03) == 1 then
		if buffer:len() < 6 then
			pinfo.desegment_len = DESEGMENT_ONE_MORE_SEGMENT
			return -1
		end

		pktbeg = 6
		pktlen = buffer(2, 4):uint()
	else
		pktbeg = 4
		pktlen = buffer(2, 2):uint()
	end

	if buffer:len() - pktbeg < pktlen then
		pinfo.desegment_len = pktlen - (buffer:len() - pktbeg)
		return -1
	end

	local subtree = tree:add(aap_protocol, buffer(0, pktbeg + pktlen))
	subtree:add(channel_id, buffer(0, 1))

	local flags = subtree:add(flags, buffer(1, 1))
	flags:add(fraginfo, buffer(1, 1))
	flags:add(msgtype, buffer(1, 1))
	flags:add(encflag, buffer(1, 1))

	if buffer(0, 1):uint() == 0 or bit.band(buffer(1, 1):uint(), 0x04) ~= 0 then
		pinfo.private["channel_id"] = "0"
	else
		pinfo.private["channel_id"] = tostring(buffer(0, 1):uint())
	end

	subtree:add(length, buffer(2, pktbeg - 2))

	if not enc then
		subtree:add(controltype, buffer(pktbeg, 2))
		local msg_id = buffer(pktbeg, 2):uint()

		if msg_id == 1 or msg_id == 2 then
			local vertree = subtree:add(verfield, buffer(pktbeg + 2, 4))
			vertree:add(verfieldmaj, buffer(pktbeg + 2, 2))
			vertree:add(verfieldmin, buffer(pktbeg + 4, 2))
		end

		if msg_id == 3 then
			tls_dissector:call(buffer(pktbeg + 2, pktlen - 2):tvb(), pinfo, tree)
		else
			dissect_payload(buffer(pktbeg, pktlen), pinfo, tree)
		end
	else
		tls_dissector:call(buffer(pktbeg, pktlen):tvb(), pinfo, tree)
	end

	return pktbeg + pktlen
end

function aap_protocol.dissector(buffer, pinfo, tree)
	pinfo.cols.protocol = aap_protocol.name
	if buffer:len() == 0 or buffer:len() ~= buffer:reported_len() then
		return 0
	end

	local begin = 0
	while begin < buffer:len() do
		local consumed = dissect_packet(buffer(begin), pinfo, tree)
		if consumed == -1 then
			pinfo.desegment_offset = 0
			return
		end
		begin = begin + consumed
	end
end

local tcp_port = DissectorTable.get("tcp.port")
tcp_port:add(5277, aap_protocol)

local aappbuf_protocol = Proto("aappbuf", "Android Auto Protobuf")

local aappbuf_type = ProtoField.uint16("aappbuf.controlmsgtype", "Control message type", base.HEX, packettypes)
aappbuf_protocol.fields = { aappbuf_type }

function aappbuf_protocol.dissector(buffer, pinfo, tree)
	pinfo.cols.protocol = aappbuf_protocol.name
	if buffer:len() == 0 or buffer:len() ~= buffer:reported_len() or pinfo.private['channel_id'] == nil then
		return 0
	end

	if pinfo.private['channel_id'] == "0" then
		local subtree = tree:add(aappbuf_protocol, buffer)
		subtree:add(aappbuf_type, buffer(0, 2))
	end

	dissect_payload(buffer, pinfo, tree)
end

local tls_port = DissectorTable.get("tls.port")
tls_port:add(5277, aappbuf_protocol)