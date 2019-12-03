from TraceClientThread import TraceClientThread
from TraceClientThread import CfgInfo
from TraceClientThread import Timer
import time
import trace_pb2

g_op_type = {'TRACE_UID':1, 'TRACE_OTHER':2}

class MyTraceClient(TraceClientThread):
    def __init__(self, name, timer):
        TraceClientThread.__init__(self, name, timer)

    def check(self, cfg, checkif):
        print('my own check')
        if cfg.p1 == g_op_type['TRACE_UID']:
            return cfg.p2 == checkif
        elif cfg.p1 == g_op_type['TRACE_OTHER']:
            return cfg.p3 == checkif


    def handle_cfg(self, msg):
        print('my own handle cfg')
        resp = trace_pb2.CfgInfoReq()
        resp.ParseFromString(msg)
        cfg = CfgInfo(resp.session_id, resp.p1, resp.p2, resp.p3)
        TraceClientThread.save_cfg(self, cfg)
        TraceClientThread.tlog_session(self, resp.session_id, 'save cfg success, p1:{}, p2:{}, p3:{}'
                .format(resp.p1, resp.p2, resp.p3))

def main():
    timer = Timer()
    timer.daemon = True
    myClient = MyTraceClient('MyTraceClient', timer)
    myClient.daemon = True
    myClient.start()
    while True:
        myClient.tlog_if(2, '{} is traced'.format(2))
        time.sleep(1)

if __name__== "__main__":
    main()
