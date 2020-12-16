from datetime import datetime
import threading

class Logger:
    _instance = None

    def __init__(self):
        self.commands = {
            '17': 'Alternar Lâmpada 01',
            '18': 'Alternar Lâmpada 02',
            'Register': 'Cadastrar cliente',
            'ToggleAlarm': 'Alternar alarme'
        }
        self.out = 'log.txt'

    @classmethod
    def get_instance(cls):
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance
    
    def log_command(self,command):
        log_data = threading.Thread(target=self.log, args=('Ação executada: %s' % self.commands[command],))
        log_data.start()
    
    def log_alarm(self):
        log_data = threading.Thread(target=self.log, args=('Alarme disparou',))
        log_data.start()

    def log(self, content):
        now = datetime.now()
        timestamp = now.strftime("%d/%m/%Y %H:%M:%S")
        with open(self.out, 'a+') as f:
            f.write('[%s] - %s\n' % (timestamp, content))
