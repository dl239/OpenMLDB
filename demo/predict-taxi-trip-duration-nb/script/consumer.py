import pulsar
import sqlalchemy as db

engine = db.create_engine('openmldb:///demo_db?zk=127.0.0.1:2181&zkPath=/openmldb')
connection = engine.connect()

client = pulsar.Client('pulsar://localhost:6650')

consumer = client.subscribe('openmldb-topic', 'my-subscription')

while True:
    msg = consumer.receive()
    try:
        print("Received message '{}' id='{}'".format(msg.data(), msg.message_id()))
        value = msg.data().decode('utf-8')
        row = value.split(',')
        insert = "insert into t1 values('%s', %s, %s, %s, %s, %s, %s, %s, %s, '%s', %s);"% tuple(row)
        connection.execute(insert)
        # Acknowledge successful processing of the message
        consumer.acknowledge(msg)
    except:
        # Message failed to be processed
        consumer.negative_acknowledge(msg)

client.close()
