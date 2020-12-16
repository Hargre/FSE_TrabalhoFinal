import threading
import subprocess
import time
from logger import Logger

def trigger_alarm():
    alarm_playback = threading.Thread(target=play_alarm)
    alarm_playback.start()
    logger = Logger.get_instance()
    logger.log_alarm()

def play_alarm():
    alarm_playback = subprocess.Popen(['omxplayer', 'media/alarm.mp3'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=True)
    time.sleep(5)
    alarm_playback.stdin.write(b'q')