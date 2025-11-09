import lief
import struct

binary = lief.parse('desktop-head-unit')

def destatify_symbol(binary, reloc_addr, symname):
	sym = binary.get_symbol(symname)

	symbol = lief.ELF.Symbol()
	symbol.name = symname
	symbol.size = 0
	symbol.value = 0
	symbol.binding = lief.ELF.SYMBOL_BINDINGS.GLOBAL

	relocation = lief.ELF.Relocation(reloc_addr, type=lief.ELF.RELOCATION_X86_64.GLOB_DAT, is_rela=True)
	relocation.addend = 0
	relocation.symbol = symbol

	binary.patch_address(sym.value, [0xff, 0x25] + list(struct.pack('<i', reloc_addr - sym.value - 6)))
	binary.remove_static_symbol(sym)
	binary.add_dynamic_relocation(relocation)

	return reloc_addr + 8

jumptable = lief.ELF.Segment()
jumptable.type = lief.ELF.SEGMENT_TYPES.LOAD
jumptable.add(lief.ELF.SEGMENT_FLAGS.R)
jumptable.add(lief.ELF.SEGMENT_FLAGS.W)
jumptable.content = [0] * 0x1000
reloc_addr = binary.add(jumptable).virtual_address

# openh264
reloc_addr = destatify_symbol(binary, reloc_addr, 'WelsCreateDecoder')
reloc_addr = destatify_symbol(binary, reloc_addr, 'WelsDestroyDecoder')

# portaudio
reloc_addr = destatify_symbol(binary, reloc_addr, 'Pa_CloseStream')
reloc_addr = destatify_symbol(binary, reloc_addr, 'Pa_GetErrorText')
reloc_addr = destatify_symbol(binary, reloc_addr, 'Pa_Initialize')
reloc_addr = destatify_symbol(binary, reloc_addr, 'Pa_IsStreamActive')
reloc_addr = destatify_symbol(binary, reloc_addr, 'Pa_OpenDefaultStream')
reloc_addr = destatify_symbol(binary, reloc_addr, 'Pa_Sleep')
reloc_addr = destatify_symbol(binary, reloc_addr, 'Pa_StartStream')
reloc_addr = destatify_symbol(binary, reloc_addr, 'Pa_StopStream')
reloc_addr = destatify_symbol(binary, reloc_addr, 'Pa_Terminate')
reloc_addr = destatify_symbol(binary, reloc_addr, 'Pa_GetSampleSize')

# ssl (they use statically linked openssl but I'm hooking their internal c++ wrapper as it's easier)
reloc_addr = destatify_symbol(binary, reloc_addr, '_ZN10SslWrapperC1Ev')
reloc_addr = destatify_symbol(binary, reloc_addr, '_ZN10SslWrapperD1Ev')
reloc_addr = destatify_symbol(binary, reloc_addr, '_ZN10SslWrapper4initERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_S8_')
reloc_addr = destatify_symbol(binary, reloc_addr, '_ZN10SslWrapper8shutdownEv')
reloc_addr = destatify_symbol(binary, reloc_addr, '_ZN10SslWrapper9handshakeEPKvmP8IoBuffer')
reloc_addr = destatify_symbol(binary, reloc_addr, '_ZN10SslWrapper10verifyPeerEv')
reloc_addr = destatify_symbol(binary, reloc_addr, '_ZN10SslWrapper25encryptionPipelineEnqueueEPvm')
reloc_addr = destatify_symbol(binary, reloc_addr, '_ZN10SslWrapper25encryptionPipelineDequeueEPvm')
reloc_addr = destatify_symbol(binary, reloc_addr, '_ZN10SslWrapper25decryptionPipelineEnqueueEPvm')
reloc_addr = destatify_symbol(binary, reloc_addr, '_ZN10SslWrapper25decryptionPipelineDequeueEPvm')

# add some exports for internal input functions (eg. they can be used to inject true multitouch input, or gps data, etc.)
binary.add_dynamic_symbol(binary.get_symbol('_ZN12SensorSource17reportCompassDataEi'))
binary.add_dynamic_symbol(binary.get_symbol('_ZN12SensorSource23reportDrivingStatusDataEi'))
binary.add_dynamic_symbol(binary.get_symbol('_ZN12SensorSource18reportLocationDataEmiibibibibi'))
binary.add_dynamic_symbol(binary.get_symbol('_ZN12SensorSource19reportNightModeDataEb'))
binary.add_dynamic_symbol(binary.get_symbol('_ZN12SensorSource15reportSpeedDataEibbbi'))
binary.add_dynamic_symbol(binary.get_symbol('_ZN12SensorSource18reportTollCardDataEb'))
binary.add_dynamic_symbol(binary.get_symbol('_ZN12SensorSource19reportCompassData3DEibibi'))
binary.add_dynamic_symbol(binary.get_symbol('_ZN11InputSource11reportTouchEmiPKjS1_S1_ii'))
binary.add_dynamic_symbol(binary.get_symbol('_ZN11InputSource14reportRelativeEmji'))
binary.add_dynamic_symbol(binary.get_symbol('_ZN11InputSource9reportKeyEmjbj'))
binary.add_dynamic_symbol(binary.get_symbol('_ZN11InputSource14reportAbsoluteEmji'))
binary.add_dynamic_symbol(binary.get_symbol('_ZN11InputSource14reportTouchPadEmiPKjS1_S1_ii'))

# they also use statically linked sdl but we don't need to hook these as these can be replaced using SDL_DYNAMIC_API mechanism

binary.write('patched')
