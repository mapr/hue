#!/bin/bash 

bin=`dirname "$0"`
bin=`cd "$bin"; pwd`
export HUE_HOME=${bin}/..

#
# activate the Python virtual environment
#
source $HUE_HOME/build/env/bin/activate
export LD_LIBRARY_PATH=$HUE_HOME/build/env/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/opt/mapr/lib:$JAVA_HOME/jre/lib/amd64/server:$LD_LIBRARY_PATH

# get arguments
command=$1
shift
startStop=$1
shift

#file client impersonation
MAPR_IMPERSONATION_ENABLED=" "
export MAPR_IMPERSONATION_ENABLED

HUE_PID_DIR=$HUE_HOME/pids
if [ "$HUE_IDENT_STRING" = "" ]; then
  export HUE_IDENT_STRING=`id -nu`
fi

# get log directory
if [ "$HUE_LOG_DIR" = "" ]; then
  export HUE_LOG_DIR="$HUE_HOME/logs"
fi
mkdir -p "$HUE_LOG_DIR"

if [ "$HUE_PID_DIR" = "" ]; then
  export HUE_PID_DIR="$HUE_HOME/pids"
fi
mkdir -p "$HUE_PID_DIR"

log=$HUE_LOG_DIR/hue-$HUE_IDENT_STRING-$command-$HOSTNAME.out
pid=$HUE_PID_DIR/hue-$HUE_IDENT_STRING-$command.pid

case $startStop in
  (start)
  if [ -f $pid ]; then
    if kill -0 `cat $pid` > /dev/null 2>&1; then
      echo Hue web server running as process `cat $pid`.  Stop it first.
      exit 1
    fi
  fi
  cd $HUE_HOME
  nohup $HUE_HOME/build/env/bin/hue $command >> "$log" 2>&1 < /dev/null &
  echo $! > $pid
  echo "`date` $command started, pid `cat $pid`" >> "$log" 2>&1 < /dev/null
    ;;

  (stop)
  if [ -f $pid ]; then
    if kill -0 `cat $pid` > /dev/null 2>&1; then
      echo stopping $command
      kill `cat $pid`
      echo "`date` $command stopped, pid `cat $pid`" >> "$log" 2>&1 < /dev/null
    else
      echo no $command to stop
    fi
  else
    echo no $command to stop
  fi
    ;;

  (status)
  if [ -f $pid ]; then
    if kill -0 `cat $pid`; then
      echo $command  running as process `cat $pid`.
      exit 0
    fi
    echo $pid exists with pid `cat $pid` but no $command.
    exit 1
  fi
  echo $command not running.
  exit 1

  ;;

  (*)
    echo $usage
    exit 1
    ;;

esac


#
# deactivate the Python virtual environment
#
deactivate

