# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License. See License.txt in the project root for
# license information.
import threading
from concurrent.futures import Future

class RequestLedger:
    def __init__(self):
        self.lock = threading.Lock()
        self.pending = {}   # id -> Future
    
    def get_response_future(self, corr_id):
        with self.lock:
            response_future = Future()
            self.pending[corr_id] = response_future
        return response_future

    def respond_to_request(self, corr_id, value):
        with self.lock:
            self.pending[corr_id].set_result(value)