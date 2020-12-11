import smbus2
import bme280

class Climate:
    def __init__(self):
        self.i2c = 1
        self.address = 0x76
        self.bus = smbus2.SMBus(self.i2c)
        self.calibration_params = bme280.load_calibration_params(self.bus, self.address)

    def read_data(self):
        data = bme280.sample(self.bus, self.address, self.calibration_params)
        # print("%.2f %.2f" % (data.temperature, data.humidity))
        return data
    