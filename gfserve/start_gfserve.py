#!/usr/bin/python2.4
import os
import sys
import signal
import os.path
import popen2
import grp, pwd

GFSERVE = "DISPLAY=:9 python gfserve.py &"
GFPIDFILE = "gfserve.py.pid"


XVFB = "/usr/bin/env Xvfb :9 -screen 0 1024x1024x24"
XVFBPIDFILE = "%s.xvfb.pid" % (__file__,)

SSHTUNNEL = "ssh -N -R 9000:localhost:9000 workspace@amb25.stccmop.edu"
SSHPIDFILE = "/home/howew/.sshtunnel.pid"

USER = "howew"
GROUP = "howew"

def restart(cmd, pidfile):
  stop(cmd, pidfile)
  return start(cmd, pidfile)

def start(cmd, pidfile):
  if os.path.exists(pidfile):
    raise IOError("%s exists, already started?" % (pidfile,))

  si = file('/dev/null', 'r')
  so = file('/dev/null', 'a+')

  out = sys.stdout
  out.write("Starting: " + cmd + "\n")
  #out.flush()

  pid = os.fork()

  sys.exit()
  if pid > 0:
    sys.exit(0)
    
  try:
    os.setsid()
  
    # redirect IO
    #os.dup2(si.fileno(), sys.stdin.fileno())
    #os.dup2(so.fileno(), sys.stdout.fileno())
    #os.dup2(so.fileno(), sys.stderr.fileno())

    #Perhaps change to http://antonym.org/node/100 ? - JGR
    os.setgid(grp.getgrnam(GROUP)[2])
    os.setuid(pwd.getpwnam(USER)[2])

    proc = popen2.Popen3(cmd)

    pf = file(pidfile, "w")
    pf.write(str(proc.pid))
    pf.close()

  except:
    out.write("    [FAILURE]\n")
    e,v,t = sys.exc_info()
    raise e,v,t
  
def stop(cmd, pidfile):
  if os.path.exists(pidfile):
    pf = file(pidfile, "r")
    pid = int(pf.read())
    pf.close()
    try:
      print "stopping %s" % (cmd,)
      os.kill(pid, signal.SIGKILL)
    except OSError:
      print "Warning: %s exists, but process %s not running." % (pidfile,pid)
    os.remove(pidfile)
  else:
    print "pidfile %s not found; is %s running?" % (pidfile, cmd)

def usage():
  print "Usage:"
  print "start_gfserve.py {start|stop|restart}"
  

if __name__ == '__main__':
  cmds = [XVFB, GFSERVE]
  pidfiles = [XVFBPIDFILE, GFPIDFILE]
  if len(sys.argv) < 2:
    for c, pf in zip(cmds, pidfiles):
      restart(c, pf)
  else:
    cmd = sys.argv[1]
    if   cmd == 'stop':
      for c, pf in zip(cmds, pidfiles):
        stop(c, pf)
    elif cmd == 'start':
      for c, pf in zip(cmds, pidfiles):
        start(c, pf)
    elif cmd == 'restart':
      for c, pf in zip(cmds, pidfiles):
        restart(c, pf)
    else:
      usage()
      

