class HouseState:
    _instance = None

    def __init__(self):
        self.inputs = {
            "Presence01": 0,
            "Presence02": 0,
            "Open01": 0,
            "Open02": 0,
            "Open03": 0,
            "Open04": 0,
        }
        self.alarm = False

    @classmethod
    def get_instance(cls):
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    def did_state_change(self, readings):
        for reading in readings:
            device = self.inputs.get(reading['name'])
            if device is not None and device != reading['value']:
                return True
        return False

    def any_inputs_active(self):
        for device in self.inputs:
            if self.inputs[device] == 1:
                return True
        return False

    def set_state(self, readings):
        for reading in readings:
            device = self.inputs.get(reading['name'])
            if device is not None:
                self.inputs[reading['name']] = reading['value']

