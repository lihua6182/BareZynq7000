project("middleware" C ASM)

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
