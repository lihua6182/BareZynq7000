
add_library(lzma INTERFACE)

target_sources(lzma
    INTERFACE
    lzma/lzma_stream.c
    lzma/sdk/LzmaDec.c
)

target_include_directories(lzma
    INTERFACE
    lzma
    lzma/sdk
)

add_library(cherryusb INTERFACE)

target_sources(cherryusb
    INTERFACE
    cherryusb/core/usbd_core.c
    cherryusb/class/cdc/usbd_cdc_acm.c
    cherryusb/port/chipidea/usb_dc_chipidea.c
)

target_include_directories(cherryusb
    INTERFACE
    cherryusb
    cherryusb/core
    cherryusb/common
    cherryusb/class/cdc
    cherryusb/port/chipidea
)
