#!/usr/bin/env python
#encoding: utf-8

import time

import vsl

if __name__ == '__main__':
    while 1:
        l = vsl.next_log()
        if l:
            print l
        else:
            time.sleep(0.001)

