ChuuniTracker Firmware
^^^^^^^^^^^^^^^^^^^^^^^

For a custom board with an ESP32-S2, BNO085 IMU, and ISL6292 Li-ion Charger.

Pinouts for ChuuniTracker as of PCB revision v1.5.1.

* IO0 = Boot0 Button
* IO5 = Voltage Divider
* IO6 = Button (Opt)
* IO8 = TCA9548 RST pin
* IO16 = BNO085 Reset (Onboard)
* IO21 = BNO085 Interrupt (Onboard)
* IO33 = SDA_EXT
* IO34 = SCL_EXT
* IO35 = SDA_INT
* IO36 = SCL_INT
* IO37 = VCCIO Enable (Required HIGH for SDA_EXT/SCL_EXT)
* IO38 = 1-bit mem for reset
* IO45 = Status NeoPixel

v1.5.1 only (unreleased):
* IO9 = I2C_EN
* IO10 = I2C_READY

In heavy development, will update README when more work is done.
