import sys
import win32event
import win32api
import winerror
from gui import create_gui
from config import MUTEX_NAME

def check_single_instance():
    """Check if another instance is running using a mutex. Exit if already running."""
    mutex = win32event.CreateMutex(None, False, MUTEX_NAME)
    last_error = win32api.GetLastError()
    if last_error == winerror.ERROR_ALREADY_EXISTS:
        sys.exit(0)
    return mutex

def main():
    # Ensure only one instance runs
    mutex = check_single_instance()
    
    # Start the GUI
    create_gui()
    
if __name__ == "__main__":
    main()