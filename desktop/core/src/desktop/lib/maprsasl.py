import sys
import os
PROTOBUF_LIB_PATH = '/build/env/build/protobuf'
SECURITY_LIB_PATH = '/build/env/lib'
sys.path.append(os.getcwd() + PROTOBUF_LIB_PATH)
sys.path.append(os.getcwd() + SECURITY_LIB_PATH)
import security
import security_pb2
import base64

CONF_FILE = '/opt/mapr/conf/mapr-clusters.conf'
def get_cluster_name():
    return open(CONF_FILE, 'r').read().split()[0]

class MaprSasl(object):

    def __init__(self):
        pass

    def init(self):
        pass

    def setAttr(self, name, attr):
        pass

    def get_init_response(self):
        serverKeyBytes = security.GetTicketAndKeyForClusterInternal(get_cluster_name(), 1)
        tk = security_pb2.TicketAndKey()
        tk.ParseFromString(serverKeyBytes)

        self.randomNumber = security.GenerateRandomNumber()
        import struct
        encr = security.Encrypt(tk.userKey.key, struct.pack('l', self.randomNumber)[::-1])
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

    def encode(self, buffer):
        success = True
        encoded = buffer
        return success, encoded

    def getError(self):
        return -1

    def step(self, payload):
        token = payload
        challenge = base64.b64decode(token)
        decodedResponse = security.Decrypt(self.tk.userKey.key, challenge)
        authResponse = security_pb2.AuthenticationResp()
        authResponse.ParseFromString(decodedResponse)
        result = authResponse.challengeResponse == self.randomNumber
        return result, ''