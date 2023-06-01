# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License. See License.txt in the project root for
# license information.
import threading
import logging
from typing import Dict, List, Callable, Any, Generic, TypeVar, Optional
logger = logging.getLogger(__name__)

T = TypeVar("T")


class IncomingAckList(Generic[T]):
    """
    Dictionary-like object which supports the concept of "waiting" for a specific key to be
    set.  This way, code that needs to wait for a specific PUBACK or SUBACK to be returned
    can easily be written using `get_next_item` with a specific `key` value.

    These "wait" operations are done in a thread-safe manner using the `Condition` class
    provided by the `threading` module.
    """

    def __init__(self) -> None:
        self.cv = threading.Condition()
        self.lookup: Dict[int, T] = {}

    def add_ack(self, key: int, value: T) -> None:
        """
        Add the ack to the dict and notify any waiting listeners
        """
        with self.cv:
            self.lookup[key] = value
            self.cv.notify_all()

    # Returns the response to the ack or None if there is
    # no response within the timeout
    def wait_for_ack(self, key: int, timeout: float) -> Optional[T]:
        """
        Wait for a given ack to be added to the dict.
        """

        def received() -> bool:
            return key in self.lookup

        with self.cv:
            if not self.cv.wait_for(received, timeout=timeout):
                return None
            else:
                try:
                    return self.lookup.pop(key)
                except KeyError:
                    # possible multiple readers.  Not really a big deal.
                    logger.warning(
                        "{} was removed from lookup list between notification and retrieval.".format(
                            key
                        )
                    )
                    return None

    def was_received(self, key: int) -> bool:
        """
        return True if a given ack was received
        """
        with self.cv:
            return key in self.lookup


message_match_predicate = Callable[[str], bool]


class IncomingMessageList(object):
    """
    thread-safe object used to keep track of incoming MQTT messages.  This object
    supports the concept of "waiting" for specific types of messages to be added to the list.
    In this way, "do-work" loops can be written which wait for specific types of messages to
    arrive and functions can be written to wait for messages like twin responses with specific
    request_id values.

    These "wait" operations are done in a thread-safe manner using the `Condition` class
    provided by the `threading` module.
    """

    def __init__(self) -> None:
        self.messages: List[Any] = []
        self.cv = threading.Condition()

    def add_message(self, message: Any) -> None:
        """
        Add a message to the message list and notify any listeners which might be
        waiting for this message.

        :param object message: The incoming message.
        """
        with self.cv:
            self.messages.append(message)
            self.cv.notify_all()

    def _pop_next(self, predicate: message_match_predicate) -> Any:
        """
        Internal function to remove and return the next message in the list
        which satisfies the passed predicate.

        :param callable predicate: Function which accepts a message object and
            Returns True if that message satisfies some condition.  When this function
            returns True for some message, that message will be removed from the list and
            returned to the caller.

        :returns: The first message in our internal list which satisfies the internal predicate.
            `None` if our internal list is empty or if no messages match the predicate.
        """

        with self.cv:
            for message in self.messages:
                if predicate(message.topic):
                    self.messages.remove(message)
                    return message
        return None

    def _wait_and_pop_next(
        self, predicate: message_match_predicate, timeout: float
    ) -> Any:
        """
        Internal function which waits until a message which matches the given predicate
        gets added to our list.

        :param callable predicate: function which accepts a message object and returns
            True if that message can be returned from this function.
        :param float timeout: Amount of time to wait before returning.

        :returns: The objet which satisfies the predicate, or `None` if no matching object
            becomes available before the timeout elapses.
        """

        with self.cv:
            return self.cv.wait_for(
                lambda: self._pop_next(predicate),  # type: ignore
                timeout=timeout,
            )

    def wait_for_message(self, timeout: float) -> bool:
        """
        Wait for the list to be not-empty.  If the list already has a
        message, return `True` immediately.  If not, then wait up to `timeout`
        seconds for an item to be added, and then return `True`.  If no item
        is added within `timeout` seconds, return False.

        :param float timeout: Amount of time to wait before returning.

        :return: `True` if the list has an item, `False` otherwise.
        """
        with self.cv:
            return self.cv.wait_for(
                lambda: len(self.messages) > 0, timeout=timeout
            )

    def pop_next_message(self, timeout: float) -> Any:
        """
        Returns the next message in the list.  If no message is in the list,
        waits for up to `timeout` seconds for one to be added.

        :param float timeout: Amount of time to wait before returning.  `0` to
            check the list and return immediately without waiting.

        :returns: The next message in the list, or `None` if no message gets
            added before the timeout elapses.
        """
        return self._wait_and_pop_next(lambda message: True, timeout=timeout)


class ConnectionStatus(object):
    """
    Connection status object which can be used to know if the transport is connected or not
    and if there was a connection error.  This object is also waitable, so users can  call
    wait_for_connected or wait_for_disconnected functions to wait for the specific states.
    """

    def __init__(self) -> None:
        self.cv = threading.Condition()
        self._connected = False
        self._connection_error: Optional[Exception] = None

    @property
    def connected(self) -> bool:
        """
        Returns True if the client is currently connected.
        """
        with self.cv:
            return self._connected

    @connected.setter
    def connected(self, connected: bool) -> None:
        with self.cv:
            self._error_state = None
            if self._connected != connected:
                self._connected = connected
                self.cv.notify_all()

    @property
    def connection_error(self) -> Exception:
        """
        Returns any connection errors that have recently happened.  This gets reset when
        setting the `connected` property.
        """
        with self.cv:
            return self.connection_error

    @connection_error.setter
    def connection_error(self, connection_error: Exception) -> None:
        if not connection_error:
            raise ValueError(
                "`connection_error` must be truthy.  Set `connected` property to clear `connection_error` property"
            )
        with self.cv:
            if self._connected or not self._connection_error:
                self._connected = False
                self._connection_error = connection_error
                self.cv.notify_all()

    def wait_for_connected(self, timeout: float = None) -> bool:
        """
        Wait for the transport to enter the connected state.
        Returns immediately if the transport is already connected.
        Raises any connection errors that happen while waiting.
        """
        with self.cv:
            self.cv.wait_for(
                lambda: self._connected or self._connection_error,
                timeout=timeout,
            )
            if self._connection_error:
                raise self._connection_error
            return self.connected

    def wait_for_disconnected(self, timeout: float = None) -> None:
        """
        Wait for the transport to enter the disconnected state.
        Returns immediately if the transport is already disconnected.
        """
        with self.cv:
            self.cv.wait_for(lambda: not self._connected, timeout=timeout)