# tts_server.py
import zmq
import threading
import time

class SimpleTTS:
    def synthesize(self, text):
        """语音合成"""
        if not text or text.strip() == "":
            return None
            
        print(f"朗读: {text}")
        return self.system_tts(text)
    
    def system_tts(self, text):
        """使用系统默认TTS"""
        try:
            import pyttsx3
            
            def speak():
                engine = pyttsx3.init()
                engine.say(text)
                engine.runAndWait()
            
            thread = threading.Thread(target=speak)
            thread.daemon = True
            thread.start()
            
            return True
            
        except Exception as e:
            print(f"TTS失败: {e}")
            return None

class TTSServer:
    def __init__(self, port=5555):
        self.port = port
        self.tts_engine = SimpleTTS()
        self.running = False
        
    def start_server(self):
        """启动ZeroMQ服务器"""
        try:
            context = zmq.Context()
            socket = context.socket(zmq.PULL)
            socket.bind(f"tcp://*:{self.port}")
            
            print("TTS服务器启动")
            print(f"端口: {self.port}")
            print("-" * 30)
            
            self.running = True
            
            while self.running:
                try:
                    message = socket.recv_string(flags=zmq.NOBLOCK)
                    self.process_message(message)
                except zmq.Again:
                    time.sleep(0.1)
                    continue
                except KeyboardInterrupt:
                    break
                except Exception as e:
                    continue
                    
        except Exception as e:
            print(f"服务器启动失败: {e}")
        finally:
            self.running = False
            if 'socket' in locals():
                socket.close()
            if 'context' in locals():
                context.term()
    
    def process_message(self, text):
        """处理接收到的文本"""
        if not text or text.strip() == "":
            return
            
        print(f"收到: {text}")
        
        thread = threading.Thread(
            target=self._synthesize,
            args=(text,)
        )
        thread.daemon = True
        thread.start()
    
    def _synthesize(self, text):
        """合成语音"""
        try:
            self.tts_engine.synthesize(text)
        except Exception:
            pass

if __name__ == "__main__":
    server = TTSServer()
    
    try:
        server.start_server()
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(f"程序异常: {e}")