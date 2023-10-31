import sqlite3
import sys
import os
import numpy as np

dap_db_fn = "dap_db.sqlite3"
dap_tbl_co_name = "coefficient_tbl"
dap_tbl_data_name = "dap_data_tbl"

DAPv_unit_str = "uGym2"
dap_tbl_co_headers = {"co_kV":"REAL", "co_mA":"REAL", "co_sec":"REAL", "co_D":"REAL", "co_size_times":"REAL"}
dap_tbl_data_headers = {"kV":"INTEGER", "uA":"INTEGER", "ms":"INTEGER", "DAP_" + DAPv_unit_str:"REAL"}
dap_tbl_data_pkeys = ("kV", "uA", "ms")

co_kV = 690.0026667
co_mA = 34689.19811
co_sec = 29106
co_D = -67396.55975
co_size_times = (np.pi * 34.5 * 33.2 / (15*15*4))

kV_range = tuple(range(60, 90+1))
uA_list = (500, 625, 800, 1000, 1250, 1575, 2000, 2500, 3150, 4000, 5000)
ms_list = (500, 550, 625, 700, 800, 900, 1000, 1100, 1250, 1400)

def connect_db(db_fpn):
    """
    Return a connection and a cursor.
    """
    conn = sqlite3.connect(db_fpn)
    cur = conn.cursor()
    return (conn, cur)

def close_db(conn, commit = True):
    if commit: conn.commit()
    conn.close()

def create_db_tbl(conn, cur, tbl_name, headers, pks = [], exist_check = True):
    """
    tbl_name: string
    headers: dictionary, col_name: data_type
    pks: list of string

    CREATE TABLE IF NOT EXISTS tbl_name (headers[i], PRIMARY KEY(pks));
    """
    db_cmd_str = "CREATE TABLE" + " " + ("IF NOT EXISTS" if exist_check else "") + " " + tbl_name
    col_name_type_str = ','.join([col_name + ((" " + data_type) if data_type else "") \
                                for (col_name, data_type) in headers.items()])
    primary_keys_str = ("PRIMARY KEY" + "(" + ",".join(pks) + ")") if pks else ""
    db_cmd_str = db_cmd_str + " (" + col_name_type_str + (("," + primary_keys_str) if primary_keys_str else "") + ");" 
    print(db_cmd_str)
    cur.execute(db_cmd_str)

def write_to_db_tbl(conn, cur, tbl_name, cols, vals, commit = True):
    """
    cols: list of col names. may be empty.
    vals: list of list, each is a series of values.
    """
    if len(vals) <= 0: return
    db_cmd_str = "INSERT INTO" + " " + tbl_name
    col_str = ("(" + ",".join(cols) + ")") if cols else ""
    #value_str_arr = [("(" + (','.join(d)) + "),") for d in vals]
    value_str_arr = vals 
    place_h_str = "(" + ("?," * len(vals[0]))[:-1] + ")"
    db_cmd_str = db_cmd_str + " " + col_str + " " + "VALUES" + " " + place_h_str
    cur.executemany(db_cmd_str, value_str_arr)
    if commit: conn.commit()

mA_uA_list = tuple((uA/1000,uA) for uA in uA_list)
sec_ms_list = tuple((ms/1000, ms) for ms in ms_list)

dap_data_fn = "dap_data.txt"

user_choose = 'y'
if(os.path.exists(dap_db_fn)):
    user_choose = input("The database file {} exists, do you want to overwrite it?(y/n)".format(dap_db_fn))
    if 'y' == user_choose:
        os.remove(dap_db_fn)
    else:
        print("new data will be inserted into current database.")

db_conn, db_cur = connect_db(dap_db_fn)
if not db_conn or not db_cur:
    print("Error: connect to {} fail!".format(dap_db_fn))
    sys.exit(-1)

create_db_tbl(db_conn, db_cur, dap_tbl_co_name, dap_tbl_co_headers)
write_to_db_tbl(db_conn, db_cur, dap_tbl_co_name, "", ((co_kV, co_mA, co_sec, co_D, co_size_times),))

create_db_tbl(db_conn, db_cur, dap_tbl_data_name, dap_tbl_data_headers, dap_tbl_data_pkeys)

dap_f = open(dap_data_fn, 'w')

print("co_kV:\t{}".format(co_kV), file = dap_f)
print("co_mA:\t{}".format(co_mA), file = dap_f)
print("co_sec:\t{}".format(co_sec), file = dap_f)
print("co_D:\t{}".format(co_D), file = dap_f)
print("co_size_times:\t{}\n".format(co_size_times), file = dap_f)

print("calculate formula:\n"
      "DAP_{} = (co_kV*kV + co_mA*mA + co_sec*sec + co_D) * co_size_times / 100000".format(DAPv_unit_str),
      file = dap_f)
print("", file = dap_f)

width = len(str(len(list(kV_range)) * len(uA_list) * len(ms_list)))
#print("No.len:{}".format(width), file = dap_f)
print("", file = dap_f)

print("kV\tuA\tms\tDAP_{}".format(DAPv_unit_str), file = dap_f)
idx = 1
DAP_v_list = []
for kV in kV_range:
    for (mA, uA) in mA_uA_list:
        for (sec, ms) in sec_ms_list:
            DAP_v = (co_kV * kV + co_mA * mA + co_sec * sec + co_D)/100000
            DAP_v = DAP_v *co_size_times 
            DAP_v_list.append((kV, uA, ms, DAP_v))
            print("{kV}\t{uA}\t{ms}\t{dap}"
                    .format(kV=kV, uA=uA, ms=ms, dap=DAP_v, width = width), 
                  file = dap_f)
            idx += 1

write_to_db_tbl(db_conn, db_cur, dap_tbl_data_name, "", DAP_v_list)

close_db(db_conn)

dap_f.close()

print("OK!")
