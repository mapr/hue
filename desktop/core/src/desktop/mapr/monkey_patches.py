import logging
import os
import threading

from desktop.lib import conf as conf_lib


LOG = logging.getLogger(__name__)

MAPR_USER = os.environ.get('MAPR_USER', 'mapr')


def run_once(func):
  def wrapper(*args, **kwargs):
    if not wrapper.has_run:
      wrapper.has_run = True
      return func(*args, **kwargs)
  wrapper.has_run = False
  return wrapper

def synchronized(func):
  lock = threading.Lock()
  def wrapper(*args, **kwargs):
    with lock:
      return func(*args, **kwargs)
  return wrapper

@synchronized
@run_once
def patch_desktop_conf():
  pass

@synchronized
@run_once
def patch_lib_conf():
  pass

@synchronized
@run_once
def patch_app_conf():
  pass
