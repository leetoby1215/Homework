import csv

filename = '107061144.csv'
data = []
SUM = 5*[0]

with open(filename) as csvfile:
    the_csv = csv.DictReader(csvfile)
    for row in the_csv:
        if (row['HUMD'] != '-99.000' and row['HUMD'] != '-999.000'):
            if (row['station_id'] == 'C0A880'):
                SUM[0] += float(row['HUMD'])
            if (row['station_id'] == 'C0F9A0'):
                SUM[1] += float(row['HUMD'])
            if (row['station_id'] == 'C0G640'):
                SUM[2] += float(row['HUMD'])
            if (row['station_id'] == 'C0R190'):
                SUM[3] += float(row['HUMD'])
            if (row['station_id'] == 'C0X260'):
                SUM[4] += float(row['HUMD'])
    if (SUM[0] == 0): data.append(['C0A880', 'None'])
    else: data.append(['C0A880', str(SUM[0])])
    if (SUM[1] == 0): data.append(['C0A880', 'None'])
    else: data.append(['C0F9A0', str(SUM[1])])
    if (SUM[2] == 0): data.append(['C0A880', 'None'])
    else: data.append(['C0G640', str(SUM[2])])
    if (SUM[3] == 0): data.append(['C0A880', 'None'])
    else: data.append(['C0R190', str(SUM[3])])
    if (SUM[4] == 0): data.append(['C0A880', 'None'])
    else: data.append(['C0X260', str(SUM[4])])

print(data)