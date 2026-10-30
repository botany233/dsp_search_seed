import sys
import threading

from .logger import Logger
log = Logger().get_logger()


def install_exception_hooks():
    def handle_exception(exc_type, exc_value, exc_traceback):
        if issubclass(exc_type, (KeyboardInterrupt, SystemExit)):
            sys.__excepthook__(exc_type, exc_value, exc_traceback)
            return
        log.error("Unhandled exception", exc_info=(exc_type, exc_value, exc_traceback))

    def handle_thread_exception(args):
        if issubclass(args.exc_type, (KeyboardInterrupt, SystemExit)):
            threading.__excepthook__(args)
            return
        handle_exception(args.exc_type, args.exc_value, args.exc_traceback)

    sys.excepthook = handle_exception
    threading.excepthook = handle_thread_exception
