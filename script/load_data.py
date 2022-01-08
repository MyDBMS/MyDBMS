import os

import csv

os.system('./bin/my_dbms < script/load_sql')

os.remove("script/load_sql")