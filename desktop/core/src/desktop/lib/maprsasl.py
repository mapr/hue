import sys
import os
PROTOBUF_LIB_PATH = '/build/env/build/protobuf'
SECURITY_LIB_PATH = '/build/env/lib'
sys.path.append(os.getcwd() + PROTOBUF_LIB_PATH)
sys.path.append(os.getcwd() + SECURITY_LIB_PATH)

def get_java_home():
  import os
  import subprocess

  def get_out(cmd):
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    out, ignore = proc.communicate()
    if proc.poll() != 0:
      raise Exception("[ERROR] JAVA_HOME not found: %s" % cmd)
    out = out.strip()
    return out

  java_bin = get_out(["bash", "-c", "type -p java"])
  java_dir = get_out(["readlink", "-f", java_bin])
  if "jre/bin/java" in java_dir:
    jdk_dir = os.path.join(java_dir, "..", "..", "..")
  elif "bin/java" in java_dir:
    jdk_dir = os.path.join(java_dir, "..", "..")
  jdk_dir = os.path.abspath(jdk_dir)
  return jdk_dir

from ctypes import *
java_home = get_java_home()
lib1 = cdll.LoadLibrary(java_home + '/jre/lib/amd64/server/libjvm.so')

def import_non_local(name, custom_name=None):
    import imp

    custom_name = custom_name or name

    f, pathname, desc = imp.find_module(name, sys.path[1:])
    module = imp.load_module(custom_name, f, pathname, desc)
    f.close()

    return module

# Choose maprsecurity.so from /opt/mapr/libexp/ directory if exists (Bug 23354)
sys.path.insert(1, "/opt/mapr/libexp/")
maprsecurity = import_non_local('maprsecurity')

from requests.auth import AuthBase
import security_pb2
import base64

CONF_FILE = '/opt/mapr/conf/mapr-clusters.conf'
def get_cluster_name():
    return open(CONF_FILE, 'r').read().split()[0]

class MaprSasl(object):

    def __init__(self):
        self.sessionKey = ''

    def init(self):
        pass

    def setAttr(self, name, attr):
        pass

    def get_init_response(self):
        serverKeyBytes = maprsecurity.GetTicketAndKeyForClusterInternal(get_cluster_name(), 1)
        tk = security_pb2.TicketAndKey()
        tk.ParseFromString(serverKeyBytes)

        self.randomNumber = maprsecurity.GenerateRandomNumber()
        import struct
        encr = maprsecurity.Encrypt(tk.userKey.key, struct.pack('l', self.randomNumber)[::-1])
        if (self.randomNumber < 0): self.randomNumber += (1 << 64)
        auth = security_pb2.AuthenticationReqFull()
        auth.encryptedRandomSecret = encr
        auth.encryptedTicket = tk.encryptedTicket

        authRequestBytes = base64.b64encode(auth.SerializeToString())
        self.tk = tk
        return authRequestBytes

    def start(self, mechanism):
        ret = True
        chosen_mech = mechanism
        initial_response = self.get_init_response()
        return ret, chosen_mech, initial_response

    def getError(self):
        return -1

    def step(self, payload):
        token = payload
        challenge = base64.b64decode(token)
        decodedResponse = maprsecurity.Decrypt(self.tk.userKey.key, challenge)
        authResponse = security_pb2.AuthenticationResp()
        authResponse.ParseFromString(decodedResponse)
        # auth = 1, auth-int = 2, auth-conf = 4
        qopInt = authResponse.encodingType
        self.sessionKey = '' if qopInt == 1 else authResponse.sessionKey.key
        result = authResponse.challengeResponse == self.randomNumber
        return result, ''

    def encode(self, data):
      import struct
      if self.sessionKey == '':
        return True, data

      encodedData = maprsecurity.Encrypt(self.sessionKey, data)
      return True, struct.pack(">I", len(encodedData)) + encodedData

    def decode(self, data):
      if self.sessionKey == '':
        return True, data

      # We need cut header (4 bytes)
      decodedData = maprsecurity.Decrypt(self.sessionKey, data[4:])
      return True, decodedData

class HttpMaprAuth(AuthBase):
    def __call__(self, request):
        mapr = MaprSasl()
        request.headers['Authorization'] = 'MAPR-Negotiate ' + mapr.get_init_response()
        return request

