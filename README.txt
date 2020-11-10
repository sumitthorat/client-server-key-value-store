Commands:

To clean the working directory:
    use command "make clean"

To run the server:
    use command "make server"

To run test client:
    use command "make run_test_client"


Key value store design:

    Cache design:
        - The number of cache lines can be configured in server_config.txt
        - The cache utilizes LRU (Least recently used) for the replacement policy.
        - There is also a function for LFU available.
        - While iterating over the cache, we find the LRU line, required line, or the line needed to be replaced in an O(n)
          where n is the number of cache lines.
        - The cache contains information such as:
            - Dirty line
            - Valid line
            - Key-value
            - Timestamp (For LRU)
            - Frequency (For LFU)
            - Each line has a write and read lock.
    
    GET call design:
        - For any GET call, the request handler first searches the cache.
            - If found, it returns the value immediately and updates the frequency and Timestamp
            - If not, it will then go to the PS and search there. Either success(200) or error of (240)

Note: Server port will be read from server_config.txt