import sqlalchemy

# connection params for the postgres database
connParams = {
    'ip': 'localhost',
    'port': 5432,
    'username': 'postgres',
    'password': 'jp123',
    'database': 'Banking',
    'schema': 'curves'
}

# function to connect a postgres database using sqlalchemy, that has as parameters the ip, port, username, password, and database name.
def connectDB(ip: str, port: int, username: str, password: str, database: str, schema: str) -> sqlalchemy.engine.base.Engine:
    connString = "postgresql://{}:{}@{}:{}/{}".format(username, password, ip, port, database)        
    engine = sqlalchemy.create_engine(connString, connect_args={'options': '-csearch_path={}'.format(schema)})    
    return engine

def get_curve_id(curve_name: str, conn: sqlalchemy.engine.base.Engine) -> int:
    try:
        sql = r"select curve_id from curves where curve_name = '{}'".format(curve_name)
        return conn.execute(sql).fetchone()[0]
    except Exception as e:
        print('get_curve_id: ' + str(e))
        return None

def get_helper_id(helper_name: str, conn: sqlalchemy.engine.base.Engine) -> int:
    try:
        sql = r"select helper_id from helpers where helper_name = '{}'".format(helper_name)
        return conn.execute(sql).fetchone()[0]
    except Exception as e:
        print('get_helper_id: ' + str(e))
        return None

def add_new_curve(curves_params: dict, conn: sqlalchemy.engine.base.Engine) -> bool:
    try:
        print(curves_params)
        sql = r"insert into curves (curve_name, day_counter, enable_extrapolation, config_date) values ('{curve_name}', '{day_counter}', {enable_extrapolation}, '{config_date}')".format(**curves_params)
        conn.execute(sql)      
        return True
    except Exception as e:
        print('add_new_curve: ' + str(e))
        return False

def add_new_helper(curve_id: int, helper_params: dict, conn: sqlalchemy.engine.base.Engine) -> bool:
    try:
        sql = r"insert into helpers (curve_id, helper_name, type) values ({curve_id}, '{helper_name}', '{type}')".format(curve_id=curve_id, 
                                                                                                                        helper_name=helper_params['helper_name'], 
                                                                                                                        type=helper_params['type'])
        conn.execute(sql)
        helper_id = get_helper_id(helper_params['helper_name'], conn)
        for field, value in zip(helper_params['fields'], helper_params['values']):
            sql = r"insert into helper_configs (helper_id, field, value) values ({}, '{}', '{}')".format(helper_id, field, value)
            conn.execute(sql)
        return True
    except Exception as e:
        print('add_new_helper: ' + str(e))
        return False

def delete_curve(curve_id: int, conn: sqlalchemy.engine.base.Engine) -> bool:
    try:
        sql = 'delete from curves where curve_id = {}'.format(curve_id)
        conn.execute(sql)
        
        sql = 'select helper_id from helpers where curve_id = {}'.format(curve_id)
        for r in conn.execute(sql):
            delete_helper(r[0], conn)        
        return True
    except Exception as e:
        print('delete_curve: ' + str(e))
        return False

def delete_helper(helper_id: int, conn: sqlalchemy.engine.base.Engine) -> bool:
    try:
        sql = 'delete from helpers where helper_id = {}'.format(helper_id)
        conn.execute(sql)
        sql = 'delete from helper_configs where helper_id = {}'.format(helper_id)
        conn.execute(sql)
        return True
    except Exception as e:
        print('delete_helper: ' + str(e))
        return False

def upload_curve(curves_params: dict, helper_params: dict, conn: sqlalchemy.engine.base.Engine) -> bool:
    if(add_new_curve(curves_params, conn)):
        curve_id = get_curve_id(curves_params['curve_name'], conn)
        for helper in helper_params:
            if(add_new_helper(curve_id, helper, conn)):
                continue
            else:
                delete_curve(curve_id, conn)
                return False
        return True
    else:
        return False

def add_tickers(tickers: str, conn: sqlalchemy.engine.base.Engine) -> bool:    
    try:
        for ticker in tickers:
            sql = r"insert into bbg_tickers (ticker) values ('{}')".format(ticker)
            conn.execute(sql)
        return True
    except Exception as e:
        print('add_tickers: ' + str(e))
        return False

def delete_tickers(tickers: str, conn: sqlalchemy.engine.base.Engine) -> bool:
    try:
        for ticker in tickers:
            sql = r"delete from bbg_tickers where ticker = '{}'".format(ticker)
            conn.execute(sql)
        return True
    except Exception as e:
        print('delete_tickers: ' + str(e))
        return False