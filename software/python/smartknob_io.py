
if __name__ == '__main__':
    import sys
    sys.exit('This is a library file to be imported into your own python scripts. It doesn\'t do anything if run directly')


from cobs import cobs
from collections import (
    defaultdict,
)
from contextlib import contextmanager
from enum import Enum
import logging
import os
from queue import (
    Empty,
    Full,
    Queue,
)
from random import randint
import serial
import serial.tools.list_ports
import sys
from threading import (
    Thread,
    Lock,
)
import time
import zlib

software_root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(software_root, 'proto_gen'))

from proto_gen import smartknob_pb2

SMARTKNOB_BAUD = 921600
PROTOBUF_PROTOCOL_VERSION = 1


class Smartknob(object):
    RETRY_TIMEOUT = 0.25

    def __init__(self, serial_instance):
        self._serial = serial_instance
        self._logger = logging.getLogger('smartknob')
        self._out_q = Queue()
        self._ack_q = Queue()
        self._next_nonce = randint(0, 255)
        self._run = True

        self._lock = Lock()
        self._message_handlers = defaultdict(list)

    def _read_loop(self):
        self._logger.debug('Read loop started')
        buffer = b''
        while True:
            buffer += self._serial.read_until(b'\0')
            if not self._run:
                return

            if not len(buffer):
                continue

            if not buffer.endswith(b'\0'):
                continue
            
            self._process_frame(buffer[:-1])
            buffer = b''

    def _process_frame(self, frame):
        try:
            decoded = cobs.decode(frame)
        except cobs.DecodeError:
            self._logger.debug(f'Failed decode ({len(frame)} bytes)')
            self._logger.debug(frame)
            return

        if len(decoded) < 4:
            return

        payload = decoded[:-4]
        expected_crc = zlib.crc32(payload) & 0xffffffff
        provided_crc = (decoded[-1] << 24) \
                        | (decoded[-2] << 16) \
                        | (decoded[-3] << 8) \
                        | decoded[-4]
        
        if expected_crc != provided_crc:
            self._logger.debug(f'Bad CRC. expected={hex(expected_crc)}, actual={hex(provided_crc)}')
            return

        message = smartknob_pb2.FromSmartKnob()
        message.ParseFromString(payload)
        self._logger.debug(message)
        if message.protocol_version != PROTOBUF_PROTOCOL_VERSION:
            self._logger.warn(f'Invalid protocol version. Expected {PROTOBUF_PROTOCOL_VERSION}, received {message.protocol_version}')

        payload_type = message.WhichOneof('payload')

        # If this is an ack, notify the write thread
        if payload_type == 'ack':
            nonce = message.ack.nonce
            self._ack_q.put(nonce)

        with self._lock:
            for handler in self._message_handlers[payload_type] + self._message_handlers[None]:
                try:
                    handler(getattr(message, payload_type))
                except:
                    self._logger.warning(f'Unhandled exception in message handler ({payload_type})', exc_info=True)
    
    def _write_loop(self):
        self._logger.debug('Write loop started')
        while True:
            data = self._out_q.get()
            # Check for shutdown
            if not self._run:
                self._logger.debug('Write loop exiting @ _out_q')
                return
            (nonce, encoded_message) = data

            next_retry = 0
            while True:
                if time.time() >= next_retry:
                    if next_retry > 0:
                        self._logger.debug('Retry write...')
                    self._serial.write(encoded_message)
                    self._serial.write(b'\0')
                    next_retry = time.time() + Smartknob.RETRY_TIMEOUT
                
                try:
                    latest_ack_nonce = self._ack_q.get(timeout=next_retry - time.time())
                except Empty:
                    latest_ack_nonce = None

                # Check for shutdown
                if not self._run:
                    self._logger.debug('Write loop exiting @ _ack_q')
                    return

                if latest_ack_nonce == nonce:
                    break
                else:
                    self._logger.debug(f'Got unexpected nonce: {latest_ack_nonce}')
    
    def _enqueue_message(self, message):
        nonce = self._next_nonce
        self._next_nonce += 1

        message.protocol_version = PROTOBUF_PROTOCOL_VERSION
        message.nonce = nonce

        payload = bytearray(message.SerializeToString())

        crc = zlib.crc32(payload) & 0xffffffff
        payload.append(crc & 0xff)
        payload.append((crc >> 8) & 0xff)
        payload.append((crc >> 16) & 0xff)
        payload.append((crc >> 24) & 0xff)

        encoded_message = cobs.encode(payload)

        self._out_q.put((nonce, encoded_message))

        approx_q_length = self._out_q.qsize()
        self._logger.debug(f'Out q length: {approx_q_length}')
        if approx_q_length > 10:
            self._logger.warning(f'Output queue length is high! ({approx_q_length}) Is the smartknob still connected and functional?')

    def set_config(self, config):
        message = smartknob_pb2.ToSmartknob()
        message.smartknob_config.CopyFrom(config)
        self._enqueue_message(message)

    def start(self):
        self.read_thread = Thread(target=self._read_loop)
        self.write_thread = Thread(target=self._write_loop)
        self.read_thread.start()
        self.write_thread.start()
    
    def shutdown(self):
        self._logger.info('Shutting down...')
        self._run = False
        self.read_thread.join()
        self._logger.debug('Read thread terminated')
        self._out_q.put(None)
        self._ack_q.put(None)
        self.write_thread.join()
        self._logger.debug('Write thread terminated')
    
    def add_handler(self, message_type, handler):
        with self._lock:
            self._message_handlers[message_type].append(handler)
        return lambda: self._remove_handler(message_type, handler)

    def _remove_handler(self, message_type, handler):
        with self._lock:
            self._message_handlers[message_type].remove(handler)

    def request_state(self):
        message = smartknob_pb2.ToSmartknob()
        message.request_state.SetInParent()
        self._enqueue_message(message)

    def hard_reset(self):
        self._serial.setRTS(True)
        self._serial.setDTR(False)
        time.sleep(0.2)
        self._serial.setDTR(True)
        time.sleep(0.2)


@contextmanager
def smartknob_context(serial_port, default_logging=True, wait_for_comms=True):
    with serial.Serial(serial_port, SMARTKNOB_BAUD, timeout=1.0) as ser:
        s = Smartknob(ser)
        s.start()

        if default_logging:
            s.add_handler('log', lambda msg: s._logger.info(f'From smartknob: {msg.msg}'))

        if wait_for_comms:
            s._logger.info('Connecting to smartknob...')
            q = Queue(1)
            def startup_handler(message):
                try:
                    q.put_nowait(None)
                except Full:
                    pass
            unregister = s.add_handler('smartknob_state', startup_handler)

            s.request_state()
            q.get()
            unregister()
            s._logger.info('Connected!')

        try:
            yield s
        finally:
            s.shutdown()


def ask_for_serial_port():
    """
    Helper function to ask which port to use via stdin
    """
    print('Available ports:')
    ports = sorted(
        filter(
            lambda p: p.description != 'n/a',
            serial.tools.list_ports.comports(),
        ),
        key=lambda p: p.device,
    )
    for i, port in enumerate(ports):
        print('[{: 2}] {} - {}'.format(i, port.device, port.description))
    print()
    value = input('Use which port? ')
    port_index = int(value)
    assert 0 <= port_index < len(ports)
    return ports[port_index].device
