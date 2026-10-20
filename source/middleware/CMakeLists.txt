project("middleware" C ASM)

# PL-bitstream codec: public-domain LZMA SDK decoder (sdk/, vendored
# verbatim) plus the streaming wrapper the ZBLZ load path in
# board_pl.c consumes (lzma_stream.c/h; encoder side:
# tools/convert_bitstream.py --codec lzma).
add_library("lzma" INTERFACE)

target_sources("lzma"
    INTERFACE
    "lzma/lzma_stream.c"
    "lzma/sdk/LzmaDec.c"
)

target_include_directories("lzma"
    INTERFACE
    "lzma"
    "lzma/sdk"
)

# Vendor rule: the SDK files stay upstream-verbatim; their vintage C
# does not chase -Wpedantic.
set_source_files_properties(
    "lzma/sdk/LzmaDec.c"
    PROPERTIES
    COMPILE_OPTIONS "-Wno-pedantic"
)

add_library("cherryusb" INTERFACE)

target_sources("cherryusb"
    INTERFACE
    "cherryusb/core/usbd_core.c"
    "cherryusb/class/cdc/usbd_cdc_acm.c"
    "cherryusb/port/chipidea/usb_dc_chipidea.c"
)

target_include_directories("cherryusb"
    INTERFACE
    "cherryusb"
    "cherryusb/core"
    "cherryusb/common"
    "cherryusb/class/cdc"
    "cherryusb/port/chipidea"
)
