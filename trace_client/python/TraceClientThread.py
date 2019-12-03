import select, socket, sys
import trace_pb2
from struct import *
from threading import *
import time
import inspect
import arrow

g_msg_type = {
    'GNS_INFO_REQ' : 0x88,
    'GNS_INFO_RESP' : 0x89,
    'CFG_INFO_REQ' : 0x8a,
    'LOGIN_REQ' : 0x8b,
    'LOGIN_RESP' : 0x8c,
    'SET_CONF_REQ' : 0x8d,
    'SET_CONF_RESP' : 0x8e,
    'LOG_INFO_REQ' : 0x8f,
    'GET_LOG_REQ' : 0x90,
    'GET_LOG_RESP' : 0x91,
    'HEARTBEAT_MSG' : 0x92,
    'CLEAR_SESSION_REQ' : 0x93
}

class MySocket:
    """demonstration class only
      - coded for clarity, not efficiency
    """

    def __init__(self, sock=None):
        if sock is None:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        else:
            self.sock = sock
        self.read_buf = bytes()

    def connect(self, host, port):
        self.sock.connect((host, port))

    def set_noblock(self):
        self.sock.setblocking(0)

    def mysend(self, msg):
        totalsent = 0
        while totalsent < len(msg):
            sent = self.sock.send(msg[totalsent:])
            if sent == 0:
                pass
                #raise RuntimeError("socket connection broken")
            totalsent = totalsent + sent

    def myreceive(self):
        MSG_HEAD_LEN = 8
        while True:
            chunk = self.sock.recv(1024)
            if chunk == b'':
                raise RuntimeError("socket connection broken")
            self.read_buf += chunk
            if len(self.read_buf) < MSG_HEAD_LEN:
                #print('{} lt header {}'.format(len(self.read_buf), MSG_HEAD_LEN))
                break
            header_pack = unpack('II', self.read_buf[:MSG_HEAD_LEN])
            body_len = socket.ntohl(header_pack[0])
            msg_id = socket.ntohl(header_pack[1])
            if len(self.read_buf) < MSG_HEAD_LEN + body_len:
                #print('body incomplete, cur_len:{}, whole_len:{}'.format(len(self.read_buf), MSG_HEAD_LEN + body_len))
                break
            body = self.read_buf[MSG_HEAD_LEN:MSG_HEAD_LEN + body_len]
            self.myparse(msg_id, body)
            self.read_buf = self.read_buf[MSG_HEAD_LEN + body_len:]
            break

    def myparse(self, msg_id, body):
        handle = self.container.myswitch(msg_id)
        handle(body)

    def fileno(self):
        return self.sock.fileno()

    def set_container(self, container):
        self.container = container

class CfgInfo():
    def __init__(self, session_id, p1, p2, p3):
        self.session_id = session_id
        self.p1 = p1
        self.p2 = p2
        self.p3 = p3

#todo connection reconnect
class TraceClientThread(Thread):
    def __init__(self, name, timer):
        Thread.__init__(self)
        self.name = name
        self.timer = timer
        self.socket = MySocket()
        self.socket.set_container(self)
        self.cfgs = []

    def run(self):
        self.socket.connect('120.25.166.70', 20009)
        self.socket.set_noblock()
        inputs = [self.socket]
        outputs = []

        self.report_self_info()
        self.timer.set_app(self)
        self.timer.start()

        while inputs:
            readable, writable, exceptional = select.select(
                inputs, outputs, inputs)
            for s in readable:
                s.myreceive()

            for s in exceptional:
                #print('exception')
                inputs.remove(s)
                s.close()

    def save_cfg(self, cfg):
        self.cfgs.append(cfg)

    def tlog(self, logtext):
        if len(self.cfgs) <= 0:
            return

        callerframerecord = inspect.stack()[1]
        frame = callerframerecord[0]
        info = inspect.getframeinfo(frame)
        for cfg in self.cfgs:
            self.upload_log(cfg.session_id, info.filename, info.lineno, info.function, logtext)

    def tlog_if(self, checkif, logtext):
        if len(self.cfgs) <= 0:
            return
        callerframerecord = inspect.stack()[1]
        frame = callerframerecord[0]
        info = inspect.getframeinfo(frame)
        for cfg in self.cfgs:
            if(self.check(cfg, checkif)):
                self.upload_log(cfg.session_id, info.filename, info.lineno, info.function, logtext)

    def tlog_session(self, session_id, logtext):
        callerframerecord = inspect.stack()[1]
        frame = callerframerecord[0]
        info = inspect.getframeinfo(frame)
        self.upload_log(session_id, info.filename, info.lineno, info.function, logtext)

    def upload_log(self, session_id, filename, lineno, function, logtext):
        now = arrow.now()
        str_time = now.format('YYYY-MM-DD HH:mm:ss SSSSSS')
        final_log_text = '{} [{}:{}({})] {}'.format(str_time, filename, lineno, function, logtext)
        log_info = trace_pb2.LogInfo()
        log_info.gns_name = self.gns_name
        log_info.ip_port = self.ip_port
        log_info.session_id = session_id
        log_info.log_text = final_log_text
        msg = log_info.SerializeToString()
        self.send_msg(msg, g_msg_type['LOG_INFO_REQ'])

    def myswitch(self, x):
        return{
            g_msg_type['CFG_INFO_REQ']: self.handle_cfg,
            g_msg_type['HEARTBEAT_MSG']: self.handle_heartbeat,
            g_msg_type['CLEAR_SESSION_REQ']: self.handle_clear_session
        }[x]

    def handle_cfg(self, msg):
        #print('leave for children to extend')
        pass

    def check(self, cfg, checkif):
        #print('leave for children to extend')
        return False

    def handle_heartbeat(self, msg):
        #print('handle heartbeat')
        pass

    def handle_clear_session(self, msg):
        #print('handle clear session')
        req = trace_pb2.ClearSessionReq()
        req.ParseFromString(msg)
        for i in range(len(self.cfgs)):
            cfg = self.cfgs[i]
            if req.session_id == cfg.session_id:
                del self.cfgs[i]

    def report_self_info(self):
        self.gns_name = 'PYTHON_TEST'
        self.ip_port = '192.168.7.126:12345'
        req = trace_pb2.GnsInfoReq()
        req.gns_name.append(self.gns_name)
        req.ip_port.append(self.ip_port)
        msg_body = req.SerializeToString()
        self.send_msg(msg_body, g_msg_type['GNS_INFO_REQ'])

    def send_heart(self):
        heartbeat = trace_pb2.HeartBeatMsg()
        heartbeat.reserve = 'nothing'
        msg_body = heartbeat.SerializeToString()
        self.send_msg(msg_body, g_msg_type['HEARTBEAT_MSG'])

    def send_msg(self, msg_body, msg_id):
        msg_id = socket.htonl(msg_id)
        msg_len = socket.htonl(len(msg_body))
        packed = pack('II{}s'.format(len(msg_body)), msg_len, msg_id, msg_body)
        self.socket.mysend(packed)

class Timer(Thread):
    def __init__(self):
        Thread.__init__(self)

    def set_app(self, comm_thread):
        self.comm_thread = comm_thread

    def run(self):
        while True:
            time.sleep(2)
            self.comm_thread.send_heart()


def main():
    timer = Timer()
    timer.daemon = True
    commThread = TraceClientThread('commThread', timer)
    commThread.daemon = True
    commThread.start()
    while True:
        time.sleep(1)

if __name__== "__main__":
    main()

