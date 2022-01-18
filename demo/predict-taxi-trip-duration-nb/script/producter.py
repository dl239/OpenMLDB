import pulsar
import time
import random
import sqlalchemy as db

client = pulsar.Client('pulsar://localhost:6650')
producer = client.create_producer('openmldb-topic')
store_and_fwd_flag = ["Y", "N"]

for i in range(10):
    row=[]
    row.append('id' + str(int(1000000 + random.random() * 1000000))) #id
    row.append(str(i % 2)) #vendor_id

    t = int(round(time.time() * 1000))
    dropoff_time = t - int(random.random() * 100000)
    duration = int(random.random() * 3000) + 200
    pickup_time = dropoff_time - duration * 1000
    row.append(str(pickup_time)) #pickup_datetime
    row.append(str(dropoff_time)) #pdropoff_datetime
    row.append(str(int(random.random() * 10 / 3 + 1))) #passenger_count
    row.append(str(-73.97746276855469 + int(random.random() * 10000) / 1000000.0)) #pickup_longitude
    row.append(str(40.7613525390625 + int(random.random() * 10000) / 1000000.0)) #pickup_latitude
    row.append(str(-73.9968032836914 + int(random.random() * 10000) / 1000000.0)) #dropoff_longitude
    row.append(str(40.743770599365234 + int(random.random() * 10000) / 1000000.0)) #dropoff_latitude
    row.append(store_and_fwd_flag[i % 2]) #store_and_fwd_flag
    row.append(str(duration)) #trip_duration
    value = (",").join(row)
    print(value)
    producer.send(value.encode('utf-8'))

client.close()
