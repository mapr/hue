import base64
import logging
import os
from requests.auth import AuthBase
import struct
from . import security_pb2

LOG = logging.getLogger(__name__)

try:
    import maprsecurity
except ImportError as err:
    # This problem typically happens only on build machines. Handle it to prevent build failure.
    msg = ("Could not import maprsecurity module.\n"
           "Any attempt to use MaprSasl will cause an exception. Please, ensure that\n"
           "1) path to maprsecurity.so in PYTHONPATH (typically $MAPR_HOME/libexp);\n"
           "2) path to libMapRClient.so and libjvm.so in LD_LIBRARY_PATH "
           "(typically $MAPR_HOME/lib and $JRE_HOME/lib/amd64/server respectively).\n"
           "{}"
           ).format(err)
    LOG.exception(msg)

MAPR_HOME = os.environ.get('MAPR_HOME', '/opt/mapr')
MAPR_CLUSTERS_CONF_FILE = os.path.join(MAPR_HOME, 'conf', 'mapr-clusters.conf')

try:
    with open(MAPR_CLUSTERS_CONF_FILE, 'r') as mapr_clusters_conf:
        MAPR_CLUSTER_NAME = mapr_clusters_conf.read().split()[0]
except IOError as err:
    # This problem typically happens only on build machines. Handle it to prevent build failure.
    msg = ("Could not open {}.\n"
           "Using default cluster name 'my.cluster.com'.\n"
           "{}"
           ).format(MAPR_CLUSTERS_CONF_FILE, err)
    LOG.exception(msg)
    MAPR_CLUSTER_NAME = 'my.cluster.com'


class MaprSasl(object):
    _QOP_AUTH = 1
    _QOP_AUTH_INT = 2
    _QOP_AUTH_CONF = 4

    def __init__(self):
        self.session_key = b''
        self.random_number = None
        self.ticket_and_key = None

    def init(self):
        pass

    def setAttr(self, name, attr):
        pass

    def get_init_response(self):
        server_key_bytes = maprsecurity.GetTicketAndKeyForClusterInternal(MAPR_CLUSTER_NAME, 1)

        self.ticket_and_key = security_pb2.TicketAndKey()
        self.ticket_and_key.ParseFromString(server_key_bytes)
        self.random_number = maprsecurity.GenerateRandomNumber()

        encrypted_random_secret = maprsecurity.Encrypt(self.ticket_and_key.userKey.key,
                                                       struct.pack('L', self.random_number)[::-1])

        auth_req = security_pb2.AuthenticationReqFull()
        auth_req.encryptedRandomSecret = encrypted_random_secret
        auth_req.encryptedTicket = self.ticket_and_key.encryptedTicket

        auth_request_bytes = base64.b64encode(auth_req.SerializeToString())
        return auth_request_bytes

    def start(self, mechanism):
        if isinstance(mechanism, str):
            mechanism = mechanism.encode('utf-8')
        return True, mechanism, self.get_init_response()

    def getError(self):
        return -1

    def step(self, payload):
        challenge = base64.b64decode(payload)
        decoded_response = maprsecurity.Decrypt(self.ticket_and_key.userKey.key, challenge)
        auth_response = security_pb2.AuthenticationResp()
        auth_response.ParseFromString(decoded_response)
        self.session_key = b'' if auth_response.encodingType == self._QOP_AUTH else auth_response.sessionKey.key
        result = auth_response.challengeResponse == self.random_number
        return result, b''

    def encode(self, data):
        if self.session_key == b'':
            return True, data

        encoded_data = maprsecurity.Encrypt(self.session_key, data)
        return True, struct.pack(">I", len(encoded_data)) + encoded_data

    def decode(self, data):
        if self.session_key == b'':
            return True, data

        # We need cut header (4 bytes)
        decoded_data = maprsecurity.Decrypt(self.session_key, data[4:])
        return True, decoded_data


class HttpMaprAuth(AuthBase):
    def __call__(self, request):
        mapr = MaprSasl()
        challenge = mapr.get_init_response().decode('utf-8')
        request.headers['Authorization'] = 'MAPR-Negotiate ' + challenge
        return request


# This library can be invoked manually for testing purposes.
def main():
    mapr = MaprSasl()
    challenge = mapr.get_init_response().decode('utf-8')
    header_string = "Authorization: MAPR-Negotiate {}".format(challenge)
    return header_string

if __name__ == "__main__":
    print(main())
