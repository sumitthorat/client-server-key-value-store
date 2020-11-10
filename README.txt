Teamname: SegFaultBusters
Members:
1) Rohit Kundu (203050030)
2) Pramod S Rao (20305R007)
3) Sumit Thorat (203050087)


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
        - When a get to a request happens, and it is not in the cache, the key-value is brought to the cache
          and response is sent for a higher locality of client requests.
    
    PUT call design:
        - For any PUT call, it is first pushed into the cache and the dirty value is set to 1. This value is later
          pushed into the PS only when it has to be evicted from the cache due to LRU. 
        - The above point elaborates the lazy update technique.

    DEL call design:
        - The key is marked as deleted in the cache line, and then it is deleted from the PS.
        - The DEL call is quite simple.
    
    Persistant storage design:
        - 
    




Note: Server port will be read from server_config.txt