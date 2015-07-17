import time
import serial
import ctypes
import pycomando


def show(bs):
    print("[echo]->%r" % bs)

        
def wait_response(func):
    """
    Decorator that waits for arduino to respond after the function is called
    """
    def inner(self, *args, **kwargs):
        func(self, *args, **kwargs)
        while self.con.inWaiting():
            self.com.handle_stream()
    inner.__name__ = func.__name__
    inner.__doc__ = func.__doc__
    return inner

    
class AutoDriverComando(object):
    def __init__(self, port='COM5', rate=9600, n_boards=1):
        self.con = serial.Serial(port, rate)
        time.sleep(1)
        self.con.setDTR(level=0)
        time.sleep(1)
        self.com = pycomando.Comando(self.con)
        self.text = pycomando.protocols.TextProtocol(self.com)
        self.cmd = pycomando.protocols.CommandProtocol(self.com)
        self.com.register_protocol(0, self.text)
        self.com.register_protocol(1, self.cmd)
        self.text.receive_message = self.show
        self.cmd.register_callback(0, self._in_waiting)
        self.cmd.register_callback(1, self._set_vars)
        self._status = {}
        self._vars = {}
        for i in range(n_boards):
            self._status[i] = None
            self._vars[i] = None

    def show(self, bs):
        print("[echo]->%r" % bs)
    
    def _wait_response(self):
        time.sleep(.1)
        while self.con.inWaiting():
            self.com.handle_stream()
        time.sleep(.1)
    
    def _in_waiting(self, cmd):
        b_ind = cmd.get_arg(ctypes.c_int16).value
        self._status[b_ind] = cmd.get_arg(bool)
    
    def _set_vars(self, cmd):
        """
        gets the vital values from arduino: max_speed, acc, dec, k, ms, low_speed
        """
        b_ind = cmd.get_arg(ctypes.c_int16).value
        self._vars[b_ind] = [cmd.get_arg(ctypes.c_int16), cmd.get_arg(ctypes.c_int16).value,
                      cmd.get_arg(ctypes.c_int16), cmd.get_arg(ctypes.c_int16).value,
                      cmd.get_arg(ctypes.c_int16), cmd.get_arg(bool)]

    def configure(self, board_ind=0):
        """
        reconfigures board with updated settings
        """
        board_ind = ctypes.c_int16(board_ind)
        self.cmd.send_command(0, (board_ind, ))

    def soft_stop(self, board_ind=0):
        """
        brings the motor to a soft stop
        """
        board_ind = ctypes.c_int16(board_ind)
        self.cmd.send_command(1, (board_ind, ))

    def hard_stop(self, board_ind=0):
        """
        brings the board to a hard stop
        """
        board_ind = ctypes.c_int16(board_ind)
        self.cmd.send_command(2, (board_ind, ))

    def release(self, board_ind=0):
        """
        releases the board
        """
        board_ind = ctypes.c_int16(board_ind)
        self.cmd.send_command(3, (board_ind, ))

    def set_max_speed(self, max_sp, board_ind=0):
        """
        sets the max speed to integer value specified
        """
        board_ind = ctypes.c_int16(board_ind)
        max_sp = ctypes.c_int16(max_sp)
        self.cmd.send_command(4, (board_ind, max_sp))

    def set_accel(self, accel, board_ind=0):
        """
        sets accel and decell to value specified
        """
        board_ind = ctypes.c_int16(board_ind)
        accel = ctypes.c_int16(accel)
        self.cmd.send_command(5, (board_ind, accel))

    def set_current(self, k_val, board_ind=0):
        """
        sets the k value (must be int between 0 and 255)
        """
        board_ind = ctypes.c_int16(board_ind)
        k_val = ctypes.c_int16(k_val)
        self.cmd.send_command(6, (board_ind, k_val))

    def set_microstepping(self, ms, board_ind=0):
        """
        sets the microstepping value int power
        i.e 7 = 2^7
        """
        board_ind = ctypes.c_int16(board_ind)
        ms = ctypes.c_int16(ms)
        self.cmd.send_command(7, (board_ind, ms))

    def low_speed_mode(self, enabled, board_ind=0):
        """
        sets low speed mode to on or off
        """
        board_ind = ctypes.c_int16(board_ind)
        enabled = bool(enabled)
        self.cmd.send_command(8, (board_ind, enabled))

    def is_moving(self, board_ind=0):
        """
        checks if board is moving (only works to
        check if the board is in the move_steps command
        """
        board_ind = ctypes.c_int16(board_ind)
        self.cmd.send_command(9, (board_ind, ))
        self._wait_response()
        tries = 0
        while tries < 5 & self._status[board_ind.value] is None:
            tries += 1
            self.cmd.send_command(9, (board_ind, ))
            self._wait_response()
        ret_val = self._status[board_ind.value]
        self._status[board_ind.value] = None
        return ret_val
        
    def current_settings(self, board_ind=0, verbose=True):
        """
        checks with the arduino and returns the current values
        """
        board_ind = ctypes.c_int16(board_ind)
        self.cmd.send_command(13, (board_ind, ))
        self._wait_response()
        tries = 0
        while (tries<5) & (self._vars[board_ind.value] is None):
            tries += 1
            self.cmd.send_command(13, (board_ind, ))
            self._wait_response()
        ret_val = self._vars[board_ind.value]
        if (verbose) & (ret_val is not None):
            print("Max Speed: {} \n Acceleration: {} \n Decelleration: {} \n"\
                   "Current: {} \n MicroStepping: {} \n LowSpeed: {}".format(ret_val[0],
                   ret_val[1],ret_val[2],ret_val[3],ret_val[4],ret_val[5]))
        self._vars[board_ind.value] = None
        return ret_val
        
    # def wait(self):
        # """
        # waits until the arduino is done with its current operation
        # """
        # self.cmd.send_command(10, (self.board_ind, ))
        # time.sleep(.5)
        # while self.con.inWaiting():
            # self.com.handle_stream()

    def rotate(self, direction, sps, board_ind=0):
        """
        rotates the motor in the direction of dir at sps steps per second
        """
        board_ind = ctypes.c_int16(board_ind)
        direction = ctypes.c_int16(direction)
        sps = ctypes.c_int16(sps)
        self.cmd.send_command(11, (board_ind, direction, sps))

    def move_steps(self, direction, steps, board_ind=0):
        """
        moves number of steps in the direction specified
        """
        board_ind = ctypes.c_int16(board_ind)
        direction = ctypes.c_int16(direction)
        steps = ctypes.c_int16(steps)
        self.cmd.send_command(12, (board_ind, direction, steps))

class AutoDriverComandoLoadCell(AutoDriverComando):
    def tare(self):
        """
        tares the scale
        """
        self.cmd.send_command(14)
        
    def reset(self):
        """
        resets the scale should be done before calibrating with no weight on it
        """
        self.cmd.send_command(15)
    
    def calibrate(self, flt_weight, int_n_avg=20):
        """
        calibrates the scale to put units of the weight that is on it.
        """
        self.cmd.send_command(16, (int_n_avg, flt_weight))
        self._wait_response()
        
    def read_force(self, n_readings=20):
        """
        reads force off the scale
        """
        self.cmd.send_command(17, (n_readings,))
        self._wait_response()