#!/usr/bin/env python3

# This module contains some tool functions used in program 'm4sMerge_bili'.
# It's a part of Bilibili Project by Yangwang.


import os
import json
import urllib.request
from socket import timeout as timeoutError
import ctypes
from threading import Thread
from time import sleep


def copy_file(file1, file2, start=0, ending=-1, append=False, buffer=1024):
    if ending >= 0 and ending <= start:
        raise ValueError("invalid start-ending of read")
    mode = 'wb'
    if os.path.exists(file1):
        if append:
            mode = 'ab'
        else:
            os.remove(file1)
    if ending < 0:
        length = os.path.getsize(file2) - start
    else:
        length = min(os.path.getsize(file2), ending) - start
    with open(file2, 'rb') as f:
        f.seek(start)
        with open(file1, mode) as _f:
            while True:
                _f.write(f.read(buffer))
                length -= buffer
                if length <= 0:
                    break


def download(url, save_path):
    '''
    :param url: direct downloadurl
    :param save_path: path to write data
    :return: infomation of download result
    '''
    try:
        with urllib.request.urlopen(url, timeout=3) as down_res:
            with open(save_path,'wb') as code:
                code.write(down_res.read())
        return True, ''
    except urllib.error.HTTPError as e:
        return False, 'Server error'
    except urllib.error.URLError:
        return False, 'Network error'
    except timeoutError:
        return False, 'Request timed out'
    except ValueError:
        return True, 'invalid url'
    except:
        return False, 'Unkown error'


class jsonCheck():
    # 去除路径下所有非法json文件的最后一个字符,
    # 针对json文件末尾多一个"}"导致解析失败的问题
    def __init__(self, path):
        self.jsonList = []
        for (a,b,c) in os.walk(path):
            for i in c:
                if i in ('entry.json','info.json') or i.split('.')[-1]=='info':
                    jsonPath = os.path.join(a, i)
                    try:
                        with open(jsonPath, 'r', encoding='utf-8') as f:
                            json.load(f)
                    except json.decoder.JSONDecodeError:
                        self.jsonList.append(jsonPath)
                    except:
                        print("<Tool:jsonCheck>Fail to check '%s'" % jsonPath)
        self.num = len(self.jsonList)

    def do(self):
        num = 0
        if self.num == 0:
            print("<Tool:jsonCheck>No abnormal json file detected.")
        else:
            for path in self.jsonList:
                with open(path, 'r', encoding='utf-8') as f:
                    data = f.read()
                with open(path, 'w', encoding='utf-8') as f:
                    f.write(data[:-1])
                num += 1
                print("<Tool:jsonCheck>Check: '%s'" % path)
        return num


def hideConsole():
    cmd_win = ctypes.windll.kernel32.GetConsoleWindow()
    if cmd_win != 0:
        ctypes.windll.user32.ShowWindow(cmd_win, 0)


def showConsole():
    cmd_win = ctypes.windll.kernel32.GetConsoleWindow()
    if cmd_win != 0:
        ctypes.windll.user32.ShowWindow(cmd_win, 1)


def hook(thread, *operation, lag=3):
    def _hook():
        while thread.is_alive():
            sleep(0.3)
        sleep(lag)
        for o in operation:
            o[0](*o[1:])
    _thread = Thread(target=_hook)
    _thread.start()

