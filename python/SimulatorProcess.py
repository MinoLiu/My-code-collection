#!/usr/bin/python3
# -*- coding: utf8 -*-

class instruction:
    def __init__(self, name, times):
        self.name = str(name)
        self.times = int(times)

    def name(self):
        return self.name
    
    def times(self):
        return self.times

    def minus(self):
        if self.times > 0:
            self.times -= 1        
            
    def __str__(self):
        return "{}({})".format(self.name, self.times)

class Process:
    def __init__(self, file):
        self.ready = []
        self.waiting = None
        self.running = None
        self.exit = []
        self.time = 0
        self.new_time = 1
        with open(str(file), 'r') as f:
            _, _limit = f.readline().strip().split(' ')
            self.limit = int(_limit)
            _name, _times = f.readline().strip().split(' ')
            self.running = instruction(_name, _times)
            for line in f.readlines():
                _name, _times = line.strip().split(' ')
                self.ready.append(instruction(_name, _times))
    
    
    def next_step(self):
        self.time += 1
        _input = input()
        if self.waiting:
            self.ready.append(self.waiting)
            self.waiting = None
        if _input == '':
            poped = False
            if self.new_time >= self.limit or self.running == None or self.running.times == 1:
                self.new_time = 0
                if self.running and self.running.times > 1:
                    self.running.minus()
                    self.ready.append(self.running)
                elif self.running and self.running.times == 1:
                    self.exit.append(self.running)
                try:
                    self.running = self.ready.pop(0)
                except:
                    self.running = None
                    self.print_status()
                    return
                poped = True
            self.new_time += 1
            if not poped:
                self.running.minus()
        else:
            self.new_time = 1
            if self.running == None and len(self.ready)>0:
                self.running = self.ready.pop(0)
            self.waiting = self.running
            self.running = None
        self.print_status()


    def print_status(self):
        print('time: {}'.format(self.time))
        string_list = [ ready.name for ready  in self.ready ]
        print('ready:',"".join(string_list))
        print('waitting: {}'.format(self.waiting))
        print('running: {}'.format(self.running))
        string_list = [ _exit.name for _exit in self.exit ]
        print('exit:',"".join(string_list))


if __name__ == '__main__':
    p = Process('input.txt')
    p.print_status()
    while(1):
        if p.running == None and len(p.ready) == 0:
            break
        p.next_step()
