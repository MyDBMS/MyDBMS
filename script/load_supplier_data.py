import os

import csv

fo = open("script/load_sql", "w")

fo.write("USE DATASET;\n")

is_str = [0, 1, 1, 0, 1, 0, 1]

length = 7

with open('./tests/data/supplier.csv') as f:
    f_csv = csv.reader(f)
    for row in f_csv:
        str = "INSERT INTO SUPPLIER VALUES ("
        cnt = 0
        for data in row:
            if (is_str[cnt] == 1):
                str += "'"
            str += data
            if (is_str[cnt] == 1):
                str += "'"
            cnt += 1
            if (cnt != length):
                str += ", "
            
        str += ");\n"
        fo.write(str)

fo.write("EXIT;\n")

fo.close()

os.system('./bin/my_dbms < script/load_sql')

os.remove("script/load_sql")