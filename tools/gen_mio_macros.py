#!/usr/bin/env python3
"""Generate per-pin MIO function-select macros for zynq_gpio.h.

Source of truth: the kernel pinctrl-zynq.c tables (Xilinx-maintained),
which encode UG585 Table 2-4 (MIO-at-a-Glance): one mux encoding per
function, valid on every pin group the function lists. Register value
= kernel mux_val << 1 (kernel MUX_SHIFT is 1; bit 0 is the tri-state).

SDIO card-detect/write-protect are muxed through the dedicated
SDIO_WP_CD_SEL registers instead, so they are not listed here.
"""

# (macro suffix, base fn, pins) - from pinctrl-zynq.c groups
PINS = []


def fn(suffix, base, pins):
    PINS.append((suffix, base, pins))


def span(a, b):
    return list(range(a, b + 1))


fn("GPIO", "GPIO", span(0, 53))
fn("QSPI_CS1", "QSPI", [0])
fn("QSPI0", "QSPI", span(1, 6))
fn("QSPI_FBCLK", "QSPI", [8])
fn("QSPI1", "QSPI", span(9, 13))
fn("ETH0", "ETH", span(16, 27))
fn("ETH1", "ETH", span(28, 39))
fn("MDIO0", "MDIO0", [52, 53])
fn("MDIO1", "MDIO1", [52, 53])
fn("USB0", "USB", span(28, 39))
fn("USB1", "USB", span(40, 51))
fn("SMC_NOR", "SMC_NOR",
   [0] + span(3, 11) + [13] + span(15, 39))
fn("SMC_NOR_ADDR25", "SMC_NOR", [1])
fn("SMC_NOR_CS1", "SMC_CS1", [1])
fn("SMC_NAND", "SMC_NAND",
   [0] + span(2, 14) + span(16, 23))
fn("SDIO0_PC", "SDIO_PC", span(0, 52)[::2])
fn("SDIO1_PC", "SDIO_PC", span(1, 53)[::2])
fn("SDIO0", "SDIO", span(16, 21) + span(28, 33) + span(40, 45))
fn("SDIO1", "SDIO", span(10, 15) + span(22, 27) + span(34, 39) + span(46, 51))
fn("CAN0", "CAN", span(10, 51)[0::2] + [])  # placeholder, replaced below
PINS.pop()
can0 = []
for g in range(11):
    can0 += [10 + 4 * g, 10 + 4 * g + 1]  # 10,11 / 14,15 / ... / 50,51
fn("CAN0", "CAN", can0)
can1 = []
for g in range(12):
    can1 += [8 + 4 * g, 8 + 4 * g + 1]  # 8,9 / 12,13 / ... / 52,53
fn("CAN1", "CAN", can1)
fn("UART0", "UART", can0)
fn("UART1", "UART", can1)
fn("I2C0", "I2C", can0)
fn("I2C1", "I2C", can1)
fn("SWDT0", "SWDT", [14, 15, 26, 27, 38, 39, 50, 51, 52, 53])
fn("TTC0", "TTC", [18, 19, 30, 31, 42, 43])
fn("TTC1", "TTC", [16, 17, 28, 29, 40, 41])
fn("SPI0", "SPI", [16, 17, 21, 18, 19, 20,
                   28, 29, 33, 30, 31, 32,
                   40, 41, 45, 42, 43, 44])
fn("SPI1", "SPI", [10, 11, 12, 13, 14, 15,
                   22, 23, 24, 25, 26, 27,
                   34, 35, 36, 37, 38, 39,
                   46, 47, 48, 49, 50, 51])

# kernel mux_val per base fn -> register bits (mux_val << 1)
BASE = {
    "GPIO": None,
    "QSPI": "MIO_PIN_OPT_LVL01_MUX",
    "ETH": "MIO_PIN_OPT_LVL01_MUX",
    "USB": "MIO_PIN_OPT_LVL11_MUX",
    "SMC_NOR": "MIO_PIN_OPT_LVL21_MUX",
    "SMC_CS1": "MIO_PIN_OPT_LVL22_MUX",
    "SMC_NAND": "MIO_PIN_OPT_LVL22_MUX",
    "SDIO_PC": "MIO_PIN_OPT_LVL23_MUX",
    "CAN": "MIO_PIN_OPT_LVL31_MUX",
    "I2C": "MIO_PIN_OPT_LVL32_MUX",
    "SWDT": "MIO_PIN_OPT_LVL33_MUX",
    "SDIO": "MIO_PIN_OPT_LVL34_MUX",
    "MDIO0": "MIO_PIN_OPT_LVL34_MUX",
    "MDIO1": "MIO_PIN_OPT_LVL35_MUX",
    "SPI": "MIO_PIN_OPT_LVL35_MUX",
    "TTC": "MIO_PIN_OPT_LVL36_MUX",
    "UART": "MIO_PIN_OPT_LVL37_MUX",
}

# collision check: same pin + same base encoding used twice
per_pin = {p: [] for p in range(54)}
for suffix, base, pins in PINS:
    for p in pins:
        assert 0 <= p <= 53, (suffix, p)
        per_pin[p].append((base, suffix))
for p, lst in per_pin.items():
    encs = [BASE[b] for b, _ in lst]
    dup = {e for e in encs if encs.count(e) > 1}
    assert not dup or dup == {None}, (p, lst)  # GPIO(0) + one 0-encoding ok

ORDER = ["GPIO", "QSPI_CS1", "QSPI0", "QSPI1", "QSPI_FBCLK",
         "ETH0", "ETH1", "MDIO0", "MDIO1", "USB0", "USB1",
         "SMC_NOR", "SMC_NOR_ADDR25", "SMC_NOR_CS1", "SMC_NAND",
         "SDIO0_PC", "SDIO1_PC", "SDIO0", "SDIO1",
         "CAN0", "CAN1", "UART0", "UART1", "I2C0", "I2C1",
         "SWDT0", "TTC0", "TTC1", "SPI0", "SPI1"]

W = len("#define GPIO_MIO53_SMC_NOR_ADDR25") + 2
out = []
for p in range(54):
    names = sorted((s for b, s in per_pin[p]), key=ORDER.index)
    out.append(f"/* MIO {p} */")
    for s in names:
        base = next(b for b, ss in per_pin[p] if ss == s)
        val = "GPIO_FN_GPIO" if base == "GPIO" else f"GPIO_FN_{base}"
        out.append(f"#define GPIO_MIO{p}_{s}".ljust(W) + val)
    out.append("")

print("\n".join(out))
