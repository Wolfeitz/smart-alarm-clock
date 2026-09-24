#!/usr/bin/env python3
"""USB setup protocol tests with synthetic data; no USB or HA access."""
import importlib.util
from pathlib import Path
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('setup', Path(__file__).with_name('configure-clock-ha.py'))
setup = importlib.util.module_from_spec(spec)
spec.loader.exec_module(setup)

class Port:
    def __init__(self, lines):
        self.lines = iter(lines)
        self.written = bytearray()
    def readline(self):
        return next(self.lines, b'')
    def write(self, data):
        self.written.extend(data)
        return len(data)

class SetupTests(unittest.TestCase):
    def setUp(self):
        self.sleep = patch.object(setup.time, 'sleep', return_value=None)
        self.sleep.start()
        self.ticks = iter(range(10000))
        self.clock = patch.object(setup.time, 'monotonic', side_effect=lambda: next(self.ticks))
        self.clock.start()
    def tearDown(self):
        self.sleep.stop()
        self.clock.stop()
    def test_tags_and_persistence(self):
        p = Port([b'SETUP_HA tag=99 saved=1\n', b'SETUP_HA tag=42 accepted=1\n', b'SETUP_HA tag=42 saved=1\n'])
        setup.save_request(p, 'synthetic-command', 'SETUP_HA', 42)
        self.assertEqual(p.written, b'synthetic-command\n')
    def test_acceptance_is_not_persistence(self):
        with self.assertRaises(setup.SetupError):
            setup.save_request(Port([b'SETUP_HA tag=42 accepted=1\n']), 'synthetic', 'SETUP_HA', 42)
    def test_failed_save(self):
        with self.assertRaises(setup.SetupError):
            setup.save_request(Port([b'SETUP_HA tag=42 accepted=1\n', b'SETUP_HA tag=42 saved=0\n']), 'synthetic', 'SETUP_HA', 42)
    def test_reordered_acknowledgments(self):
        setup.save_request(Port([b'SETUP_MEDIA tag=42 saved=1\n', b'SETUP_MEDIA tag=42 accepted=1\n']), 'synthetic', 'SETUP_MEDIA', 42)
    def test_rejected_input_and_bound(self):
        with self.assertRaises(setup.SetupError):
            setup.wait_for(Port([b'SETUP_ERROR invalid_request\n']), 'SETUP_READY ')
        with self.assertRaises(setup.SetupError):
            setup.send(Port([]), 'x' * 1900)
    def test_target_validation_and_redirect(self):
        self.assertTrue(setup.endpoint_valid('http://192.168.1.232:8123'))
        for url in ['http://user:password@host', 'http://@host', 'http://host/dashboard', 'ftp://host']:
            self.assertFalse(setup.endpoint_valid(url))
        self.assertTrue(setup.entity_valid('media_player.bedroom', 'media_player'))
        self.assertFalse(setup.entity_valid('light.bedroom', 'media_player'))
        self.assertIsNone(setup.NoRedirect().redirect_request(None, None, 302, '', {}, 'http://another-host'))

unittest.main()
