ID = 1
D0 = 6
D1 = 7
D2 = 8
D3 = 9
D4 = 10
D5 = 11
D6 = 12
D7 = 13
TIME = 0


def get_uint16(HI8, LO8):
    return 256 * HI8 + LO8


def get_int16(HI8, LO8):
    uint16 = get_uint16(HI8, LO8)
    if (uint16 >= 32768):
        uint16 -= (32768 * 2)
    return uint16


def to_int16(row, HI8_COL, LO8_COL, scaler=1):
    value = get_int16(row[HI8_COL], row[LO8_COL])
    cols = [HI8_COL, LO8_COL]
    row[max(cols)] = ''
    row[min(cols)] = value / scaler
    return row


def to_uint16(row, HI8_COL, LO8_COL, scaler=1):
    value = get_uint16(row[HI8_COL], row[LO8_COL])
    cols = [HI8_COL, LO8_COL]
    row[max(cols)] = ''
    row[min(cols)] = value / scaler
    return row

    # def parse_400(row):
    #     row = to_int16(row, D0, D1, 10)  # inlet temp
    #     row = to_int16(row, D2, D3, 10)  # outlet temp
    #     row = to_int16(row, D4, D5)      # in air temp
    #     row = to_int16(row, D6, D7)      # out air temp
    #     return row

    # def parse_0C0(row):
    #     row = to_int16(row, D1, D0, 10)   # torque request
    #     row = to_int16(row, D3, D2)
    #     row = to_int16(row, D7, D6, 10)
    #     return row

    # def parse_380(row):
    #     row = to_uint16(row, D2, D3)       # BMS status flags
    #     row = to_uint16(row, D4, D5, 100)  # pack voltage
    #     row = to_uint16(row, D6, D7)       # spi error flags
    #     return row

    # def parse_387(row):
    #     row = to_int16(row, D0, D1, 10)   # DC current (amps)
    #     print(row[D3])
    #     return row

    # def parse_0A0(row):
    #     row = to_int16(row, D1, D0, 10)  # mod a temp
    #     row = to_int16(row, D3, D2, 10)  # mod b temp
    #     row = to_int16(row, D5, D4, 10)  # mod c temp
    #     row = to_int16(row, D7, D6, 10)  # gate driver temp
    #     return row

    # def parse_0A1(row):
    #     row = to_int16(row, D1, D0, 10)  # control board temp
    #     row = to_int16(row, D3, D2, 10)  # RTD #1 temp (unused)
    #     row = to_int16(row, D5, D4, 10)  # RTD #2 temp (unused)
    #     row = to_int16(row, D7, D6, 10)  # power module junction temperature estimate

    # def parse_0A2(row):
    #     row = to_int16(row, D1, D0, 10)  # estimated coolant temp
    #     row = to_int16(row, D3, D2, 10)  # estimated hot spot temp
    #     row = to_int16(row, D5, D4, 10)  # motor temp
    #     row = to_int16(row, D7, D6, 10)  # torque shudder
    #     return row

    # # motor position info
    # def parse_0A5(row):
    #     row = to_int16(row, D1, D0, 10)  # motor angle in degrees
    #     row = to_int16(row, D3, D2)      # angular velocity in rpm
    #     row = to_int16(row, D5, D4, 10)  # electrical output frequency in hz
    #     row = to_int16(row, D7, D6, 10)  # delta resolver
    #     return row

    # def parse_0A7(row):
    #     row = to_int16(row, D1, D0, 10)  # HV DC bus voltage
    #     row = to_int16(row, D3, D2, 10)  # HV output voltage
    #     row = to_int16(row, D5, D4, 10)  # Vd (direct axis voltage)
    #     row = to_int16(row, D7, D6, 10)  # Vq (quadrature axis voltage)
    #     return row

    # def parse_401(row):
    #     row = to_uint16(row, D1, D0)       # wheel speed RR
    #     row = to_uint16(row, D3, D2)       # wheel speed RL
    #     return row

    # def parse_766(row):
    #     row = to_uint16(row, D6, D7)       # tick
    #     return row

    # def parse_500(row):
    #     row = to_uint16(row, D0, D1)      # strain gauge raw ADC reading
    #     row = to_uint16(row, D3, D2)      # wheel speed front
    #     row = to_uint16(row, D5, D4, 10)  # TC_torque_request (Nm * 10)
    #     return row

    # def parse_100(row):
    #     row = to_int16(row, D0, D1)      # ang x
    #     row = to_int16(row, D3, D2)      # ang y
    #     row = to_int16(row, D5, D4)      # ang z
    #     return row

    # def parse_101(row):
    #     row = to_int16(row, D0, D1, 100)      # accel x
    #     row = to_int16(row, D3, D2, 100)      # accel y
    #     row = to_int16(row, D5, D4, 100)      # accel z
    #     return row


def parse_row(row):
    if (row[ID] == 0x400):
        row = to_int16(row, D0, D1, 10)  # inlet temp
        row = to_int16(row, D2, D3, 10)  # outlet temp
        row = to_int16(row, D4, D5)  # in air temp
        row = to_int16(row, D6, D7)  # out air temp

    elif (row[ID] == 0x387):
        row = to_int16(row, D0, D1, 10)  # DC current (amps)

    elif (row[ID] == 0x0C0):
        row = to_int16(row, D1, D0, 10)  # torque request
        row = to_int16(row, D3, D2)
        row = to_int16(row, D7, D6, 10)

    elif (row[ID] == 0x380):
        row = to_uint16(row, D2, D3)  # BMS status flags
        row = to_uint16(row, D4, D5, 100)  # pack voltage
        row = to_uint16(row, D6, D7)  # spi error flags

    elif (row[ID] == 0x0A0):
        row = to_int16(row, D1, D0, 10)  # mod a temp
        row = to_int16(row, D3, D2, 10)  # mod b temp
        row = to_int16(row, D5, D4, 10)  # mod c temp
        row = to_int16(row, D7, D6, 10)  # gate driver temp

    elif (row[ID] == 0x0A1):
        row = to_int16(row, D1, D0, 10)  # control board temp
        row = to_int16(row, D3, D2, 10)  # RTD #1 temp (unused)
        row = to_int16(row, D5, D4, 10)  # RTD #2 temp (unused)
        row = to_int16(row, D7, D6, 10)  # power module junction temperature estimate

    elif (row[ID] == 0x0A2):
        row = to_int16(row, D1, D0, 10)  # estimated coolant temp
        row = to_int16(row, D3, D2, 10)  # estimated hot spot temp
        row = to_int16(row, D5, D4, 10)  # motor temp
        row = to_int16(row, D7, D6, 10)  # torque shudder

    elif (row[ID] == 0x0A5):
        row = to_int16(row, D1, D0, 10)  # motor angle in degrees
        row = to_int16(row, D3, D2, -1)  # angular velocity in rpm
        row = to_int16(row, D5, D4, 10)  # electrical output frequency in hz
        row = to_int16(row, D7, D6, 10)  # delta resolver

    elif (row[ID] == 0x0A7):
        row = to_int16(row, D1, D0, 10)  # HV DC bus voltage
        row = to_int16(row, D3, D2, 10)  # HV output voltage
        row = to_int16(row, D5, D4, 10)  # Vd (direct axis voltage)
        row = to_int16(row, D7, D6, 10)  # Vq (quadrature axis voltage)

    elif (row[ID] == 0x401):
        row = to_uint16(row, D1, D0)  # wheel speed RR
        row = to_uint16(row, D3, D2)  # wheel speed RL

    elif (row[ID] == 0x766):
        row = to_uint16(row, D6, D7)  # tick

    elif (row[ID] == 0x500):
        row = to_uint16(row, D0, D1)  # strain gauge raw ADC reading
        row = to_uint16(row, D2, D3)  # wheel speed front
        row = to_uint16(row, D4, D5, 10)  # TC_torque_request (Nm * 10)

    elif (row[ID] == 0x100):
        row = to_int16(row, D0, D1)  # ang x
        row = to_int16(row, D2, D3)  # ang y
        row = to_int16(row, D4, D5)  # ang z

    elif (row[ID] == 0x101):
        row = to_int16(row, D0, D1, 100)  # accel x
        row = to_int16(row, D2, D3, 100)  # accel y
        row = to_int16(row, D4, D5, 100)  # accel z

    elif (row[ID] == 0x402):
        row = to_uint16(row, D0, D1, 100)  # inlet pressure, psi*100
        row = to_uint16(row, D2, D3, 100)  # outlet pressure, psi*100

    elif (row[ID] == 0x403):
        row = to_uint16(row, D0, D1)  # rear strain gauge raw ADC

    return row


def get_lines(filepath):
    f = open(filepath, 'r')
    lines = f.readlines()
    f.close()

    del lines[0]

    unhex = lambda x: int(x, 16)
    for i in range(0, len(lines)):
        lines[i] = lines[i].split(',')

        # del lines[i][5]
        # del lines[i][4]
        # del lines[i][3]
        # del lines[i][2]
        # del lines[i][0]
        # del lines[i][9]

        for col in [ID, D0, D1, D2, D3, D4, D5, D6, D7]:
            lines[i][col] = unhex(lines[i][col])

    return lines


def to_csv(filepath, lines):
    f = open(filepath, 'w')
    for i in range(0, len(lines)):
        lines[i][ID] = f'{lines[i][ID]:x}'
        str_line = [str(val) for val in lines[i]]
        lines[i] = ','.join(str_line)
        f.write(lines[i])
        f.write('\n')

    f.close()


def __main__():
    global count

    csv_name = "003_victor"
    folder = "C:/Users/Maxwell Chen/Downloads/"

    filepath = folder + csv_name + ".csv"
    lines = get_lines(filepath)

    rollover_count = 0
    max_val = 2 ** 16
    time = 0
    last_time = 0

    for i in range(0, len(lines)):
        # print(lines[i])
        lines[i] = parse_row(lines[i])

        if (lines[i][ID] == 0x766):
            this_time = lines[i][D6]
            if (this_time < last_time):
                rollover_count += 1

            time = rollover_count * max_val + this_time
            last_time = this_time

        lines[i].append(time / 1000)

    # print(count)

    parsed_file = folder + csv_name + "_parsed.csv"
    to_csv(parsed_file, lines)

__main__()