import base64
import ctypes
import logging
import os
from puresasl import QOP, SASLError
from puresasl.mechanisms import Mechanism, mechanisms
import security_pb2
import struct

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


class MaprSaslMechanism(Mechanism):

    name = 'MAPRSASL'

    score = 100
    qops = QOP.all

    allows_anonymous = False
    uses_plaintext = False
    active_safe = True

    def __init__(self, sasl, **props):
        Mechanism.__init__(self, sasl)

        self._session_key = ''
        self._random_number = None
        self._ticket_and_key = None

    def process(self, challenge=None):
        LOG.debug("Negotiating: processing with challange of None." if challenge is None
                  else "Negotiating: processing challange with length of {}.".format(len(challenge)))

        # 1 request - Generate Authentication Request
        if not self._ticket_and_key:
            server_key_bytes = maprsecurity.GetTicketAndKeyForClusterInternal(MAPR_CLUSTER_NAME, 1)

            self._ticket_and_key = security_pb2.TicketAndKey()
            self._ticket_and_key.ParseFromString(server_key_bytes)

            self._random_number = maprsecurity.GenerateRandomNumber()
            # Workaround for a bug when GenerateRandomNumber returns long, while should return unsigned long
            if self._random_number < 0:
                self._random_number = ctypes.c_uint64(self._random_number).value

            encrypted_random_secret = maprsecurity.Encrypt(self._ticket_and_key.userKey.key,
                                                           struct.pack('L', self._random_number)[::-1])

            auth_req = security_pb2.AuthenticationReqFull()
            auth_req.encryptedRandomSecret = encrypted_random_secret
            auth_req.encryptedTicket = self._ticket_and_key.encryptedTicket

            auth_request_bytes = base64.b64encode(auth_req.SerializeToString())
            LOG.debug("Negotiating: sending response with length of {}.".format(len(auth_request_bytes)))
            return auth_request_bytes

        # 1 response - Handle Authentication Response
        # 2 request - Send an empty message as confirmation that the client accepts the server
        if challenge:
            challenge = base64.b64decode(challenge)
            decoded_response = maprsecurity.Decrypt(self._ticket_and_key.userKey.key, challenge)
            auth_response = security_pb2.AuthenticationResp()
            auth_response.ParseFromString(decoded_response)

            server_offered_qops = QOP.names_from_bitmask(auth_response.encodingType)
            self._pick_qop(server_offered_qops)

            if self.qop != QOP.AUTH:
                self._session_key = auth_response.sessionKey.key

            self._success = auth_response.challengeResponse == self._random_number

            if not self._success:
                raise SASLError()

            LOG.debug("Negotiating: sending an empty message as confirmation that the client accepts the server.")
            return ''

        # 2 response - Finally, get an empty response and mark the negotiation as complete
        else:
            if self._success:
                self.complete = True
                LOG.debug("Negotiating: negotiation is complete.")
            else:
                # Seems like the previous step was missing
                raise SASLError()

        return ''

    def wrap(self, outgoing):
        # This method got copied from maprsasl.py (maprsasl implementation for Thrift).
        # It got copied while implementing maprsasl for Zookeeper, but it's never invoked in Zookeeper client.
        # So there is no sure it works.
        if self.qop == QOP.AUTH:
            return outgoing

        encoded_data = maprsecurity.Encrypt(self._session_key, outgoing)
        payload = struct.pack(">I", len(encoded_data)) + encoded_data
        return payload

    def unwrap(self, incoming):
        # This method got copied from maprsasl.py (maprsasl implementation for Thrift).
        # It got copied while implementing maprsasl for Zookeeper, but it's never invoked in Zookeeper client.
        # So there is no sure it works.
        if self.qop == QOP.AUTH:
            return incoming

        # We need cut header (4 bytes)
        decoded_data = maprsecurity.Decrypt(self._session_key, incoming[4:])
        return decoded_data


if MaprSaslMechanism.name not in mechanisms:
    mechanisms[MaprSaslMechanism.name] = MaprSaslMechanism
