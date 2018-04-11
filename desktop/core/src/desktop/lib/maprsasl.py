import logging
import os

LOG = logging.getLogger(__name__)

try:
    import maprsecurity
except ImportError as e:
    msg = ("Could not import maprsecurity module.\n" +
           "Any attempt to use MaprSasl will cause an exception. Please, ensure that\n" +
           "1) path to maprsecurity.so in PYTHONPATH (typically $MAPR_HOME/libexp);\n" +
           "2) path to libMapRClient.so and libjvm.so in LD_LIBRARY_PATH " +
           "(typically $MAPR_HOME/lib and $JRE_HOME/lib/amd64/server respectively).\n" +
           "%s"
           ) % (e, )
    LOG.exception(msg)

from requests.auth import AuthBase
import security_pb2
import base64

MAPR_HOME = os.environ.get('MAPR_HOME', '/opt/mapr')
CONF_FILE = os.path.join(MAPR_HOME, 'conf', 'mapr-clusters.conf')

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

