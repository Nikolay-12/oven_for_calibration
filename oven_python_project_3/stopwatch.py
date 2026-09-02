import datetime

def get_current_time():
    return datetime.datetime.now().time()

def elapsed_time_in_sec(start_point, current_point):
    seconds = current_point.second - start_point.second
    minutes = current_point.minute - start_point.minute
    hours = current_point.hour - start_point.hour
    return seconds + minutes * 60 + hours * 3600