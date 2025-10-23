import logging
import colorlog
import os
import sys
from pathlib import Path

class CustomFormatter(colorlog.ColoredFormatter):

    root = Path(sys._MEIPASS) if hasattr(sys, "_MEIPASS") else Path.cwd()

    def format(self, record: logging.LogRecord) -> str:
        record.pathname = os.path.relpath(record.pathname, self.root).replace(r" ..\\..\\", "")
        msg = super().format(record)
        msg = msg.replace(",", ".", 1)

        return msg

class Logger:
    def __init__(self, name: str = "dsp_search_seed"):
        self.logger = logging.getLogger(name)
        self.logger.propagate = False
        self.logger.setLevel(logging.DEBUG)


        if not self.logger.handlers:
            # 控制台输出
            console_handler = logging.StreamHandler()
            console_formatter = CustomFormatter(
                "%(log_color)s[%(levelname)s] %(asctime)s \"%(pathname)s:%(lineno)d:\" %(message)s",
                log_colors={
                    "DEBUG": "cyan",
                    "INFO": "green",
                    "WARNING": "yellow",
                    "ERROR": "red",
                    "CRITICAL": "red,bg_white",
                },
            )
            console_handler.setFormatter(console_formatter)
            console_handler.setLevel(logging.DEBUG)
            self.logger.addHandler(console_handler)

            os.makedirs("./logs", exist_ok=True)

            file_handler = logging.FileHandler("./logs/debug.log", encoding="utf-8")
            file_formatter = CustomFormatter(
                "[%(levelname)s] %(asctime)s \"%(pathname)s:%(lineno)d:\" %(message)s",
            )
            file_formatter.no_color = True
            file_handler.setFormatter(file_formatter)
            file_handler.setLevel(logging.DEBUG)
            self.logger.addHandler(file_handler)


            

    def get_logger(self) -> logging.Logger:
        return self.logger
    
if __name__ == "__main__":
    log = Logger().get_logger()
    log.debug("This is a debug message")
    log.info("This is an info message")
    log.warning("This is a warning message")
    log.error("This is an error message")
    log.critical("This is a critical message")
