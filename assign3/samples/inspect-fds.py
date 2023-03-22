#!/usr/bin/python2

import re
import os
import sys
import subprocess

O_ACCMODE   = 00000003
O_RDONLY    = 00000000
O_WRONLY    = 00000001
O_RDWR      = 00000002
O_CREAT     = 00000100
O_EXCL      = 00000200
O_NOCTTY    = 00000400
O_TRUNC     = 00001000
O_APPEND    = 00002000
O_NONBLOCK  = 00004000
O_DSYNC     = 00010000
FASYNC      = 00020000
O_DIRECT    = 00040000
O_LARGEFILE = 00100000
O_DIRECTORY = 00200000
O_NOFOLLOW  = 00400000
O_NOATIME   = 01000000
O_CLOEXEC   = 02000000

COLORS = ['\033[91m', '\033[92m', '\033[93m', '\033[94m', '\033[95m', '\033[96m']
CLEAR_COLOR = '\033[0m'

pipe_colors = {}
next_pipe_color_index = 0
def get_pipe_color(inumber):
    global pipe_colors
    global next_pipe_color_index
    if inumber not in pipe_colors:
        pipe_colors[inumber] = COLORS[next_pipe_color_index % len(COLORS)]
        next_pipe_color_index += 1
    return pipe_colors[inumber]

def get_fd_info(pid, fd):
    target = os.path.realpath('/proc/%s/fd/%s' % (pid, fd))

    if target.startswith('/dev/pts/'):
        name = '<terminal>'
    elif target.startswith('/proc/%s/fd/pipe:' % pid):
        inumber = target[target.index('[') + 1 : target.index(']')]
        name = (get_pipe_color(inumber) + '<pipe #%s>' % inumber + CLEAR_COLOR)
    else:
        name = target.split('/')[-1]

    with open('/proc/%s/fdinfo/%s' % (pid, fd), 'r') as f:
        fdinfo = f.read()
    flags = int(re.search(r'flags:\s*(\d+)', fdinfo).group(1), 8)   # 8 for octal
    access = '(read)'
    if flags & O_WRONLY:
        access = '(write)'
    if flags & O_RDWR:
        access = '(read/write)'

    return {
        'fd': fd,
        'name': name,
        'access': access,
    }

def print_process_fds(pid):
    print '========== %s (pid %s, ppid %s) ========== ' \
            % (get_process_name(pid), pid, get_ppid(pid))
    try:
        fds = sorted(
                [get_fd_info(pid, fd) for fd in os.listdir('/proc/%s/fd' % pid)],
                key=lambda x: int(x['fd']))
        for fd in fds:
            print '{:<4}{:<18}{}'.format(fd['fd'], fd['access'].ljust(15), fd['name'])
    except OSError:
        print ('Warning: could not inspect file descriptors for this process! '
                'It might have exited just as we were about to look at its fd table, '
                'or it might have exited a while ago and is waiting for the parent '
                'to reap it.')

def get_process_name(pid):
    return subprocess.check_output(['ps', '--pid', pid, '-o', 'cmd=']).split()[0]

def get_ppid(pid):
    return subprocess.check_output(['ps', '--pid', pid, '-o', 'ppid=']).strip()

def list_child_processes(pid):
    try:
        return subprocess.check_output(['ps', '--ppid', pid, '-o', 'pid=']).split()
    except subprocess.CalledProcessError:
        return []

def search_running_command(cmd):
    try:
        return subprocess.check_output(['pgrep', '-U', os.getlogin(), cmd],
                stderr=subprocess.STDOUT).split()[0]
    except subprocess.CalledProcessError:
        return None

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print 'Usage: python %s <pid or name of program>' % sys.argv[0]
        sys.exit(1)

    # First attempt to use the name of a program
    pid = search_running_command(sys.argv[1])
    if not pid:
        # That didn't work... let's try using supplied arg as a pid
        pid = sys.argv[1]
        try:
            subprocess.check_output(['ps', '--pid', pid],
                    stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError:
            print '"%s" doesn\'t seem to be a valid running program name or PID!' % sys.argv[1]
            sys.exit(1)

    print_process_fds(pid)
    for child in list_child_processes(pid):
        print_process_fds(child)
