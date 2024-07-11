import json


def find_json(json_path):
    """
    Find token.json in given path.

    Args:
        json_path (string): The file path of token.json.

    Returns:
        json_data (structure): The json data structure.
    """
    try:
        with open(json_path, 'r') as f:
            json_data = json.load(f)

    except FileNotFoundError:
        print(json_path + " is not find, abort.")
        exit(1)

    except json.JSONDecodeError:
        print(json_path + " parse error, abort.")
        exit(1)

    return json_data


def get_json_data(json_data):
    """
    Get the json data from token.json.

    Args:
        json_data (structure): The data structure of json.

    Returns:
        [ start_addr, sizeByte, write_data ]: The array of json data.
    """
    startAddr = json_data.get("offset")
    sizeByte = json_data.get("sizeB")
    writeData = json_data.get("data")

    return [ startAddr, sizeByte, writeData ]


def covertData(sizeB, data):
    _TMP_ = data.split("0x")

    if len(_TMP_) == 1:
        return [format(ord(char), '02x') for char in _TMP_[0]]
    else:
        if (len(_TMP_[1:]) != sizeB):  # Check if the sizeB matches the length of the data.
            print("The len of data do not match the input sizeB, abort.")
            exit(1)
        
        return [Str.replace(',', '') for Str in _TMP_[1:]]


def gen_token_bin(token_bin_path, addr, sizeB, data):
    """
    Generate token.bin according to the data of token.json.

    Args:
        token_bin_path (string): The path of token.bin for new generate.
        addr (int): Location of written data.
        data (string): The data to be written.
    """
    try:
        with open(token_bin_path, 'r+b') as f:
            f.seek(addr)

            covertArray = covertData(sizeB, data)
            for val in covertArray:
                f.write(bytes.fromhex(val))

    except IOError:
        print("An error occurred while open or write to the bin, abort.")
        exit(1)


def main():
    """
    Set the basic working directory in 
    'examples/lighting-app/telink/...' 
    for gen_token.py
    """

    token_json_path = "./token.json"

    token_bin_path = "./token.bin"

    token_bin_size = 4096  # 4KB

    # Create a bin with all values of '0xff'
    with open(token_bin_path, 'wb+') as f: 
        f.write(bytes([0xff] * token_bin_size)) 

    # Find the token.json.
    _JSON_DATA_ = find_json(
        token_json_path
    )

    for json_data in _JSON_DATA_:

        # Get the json data.
        _WRITE_BIN_ = get_json_data(
            json_data
        )

        # Check the legitimacy of input parameters.
        addr_len = len(_WRITE_BIN_[0])
        data_len = len(_WRITE_BIN_[2])

        if (addr_len == 0) or (_WRITE_BIN_[1] == 0) or (data_len == 0):  # Ensure input has value.
            print(
                "There is a null for write_addr_len(%d) or sizeB(%d) or json_data_len(%d), continue."
                % (addr_len, _WRITE_BIN_[1], data_len)
            )
            continue
        

        write_addr = int(_WRITE_BIN_[0], 0)
        if (write_addr < 0) or (write_addr > token_bin_size):  # Ensure the write addr for input has valid.
            print(
                "The write_addr(0x%X) exceeds the size of bin(%dB)." 
                % (write_addr, token_bin_size)
            )
            continue

        # Generate token bin according to the json
        gen_token_bin(
            token_bin_path, 
            write_addr, 
            _WRITE_BIN_[1], 
            _WRITE_BIN_[2]
        )


    print("Generate successfully for the token.bin.")


if __name__ == '__main__':
    main()
