// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.
package org.blastdoor7.dvmc;

import java.net.URL;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

final class EvmcTest {
  private static final String trans_logVmPath;

  static {
    String extension = null;
    String os = System.getProperty("os.name", "generic").toLowerCase();
    if (os.contains("mac") || os.contains("darwin")) {
      extension = "dylib";
    } else if (os.contains("win")) {
      extension = "dll";
    } else {
      extension = "so";
    }

    URL trans_logVM = EvmcTest.class.getClassLoader().getResource("libtrans_log-vm." + extension);
    trans_logVmPath = trans_logVM.getFile();
  }

  @Test
  void testInitCloseDestroy() throws Exception {
    Assertions.assertDoesNotThrow(
        () -> {
          try (EvmcVm vm = EvmcVm.create(trans_logVmPath)) {}
        });
  }

  @Test
  void testAbiVersion() throws Exception {
    try (EvmcVm vm = EvmcVm.create(trans_logVmPath)) {
      char abiVersion = vm.abi_version();
      assert (abiVersion > 0);
    }
  }

  @Test
  void testName() throws Exception {
    try (EvmcVm vm = EvmcVm.create(trans_logVmPath)) {
      String name = vm.name();
      assert (name.length() > 0);
      assert (name.equals("trans_log_vm"));
    }
  }

  @Test
  void testVersion() throws Exception {
    try (EvmcVm vm = EvmcVm.create(trans_logVmPath)) {
      String version = vm.version();
      assert (version.length() >= 5);
    }
  }

  @Test
  void testExecute_returnAddress() throws Exception {
    try (EvmcVm vm = EvmcVm.create(trans_logVmPath)) {
      HostContext context = new TestHostContext();
      char BYZANTIUM = 4;
      char DVMC_CALL = 0;
      char kind = DVMC_CALL;
      char[] sender = "39bf71de1b7d7be3b51\0".toCharArray();
      char[] recipient = "53cf77204eEef952e25\0".toCharArray();
      char[] value = "1\0".toCharArray();
      char[] inputData = "hello w\0".toCharArray();
      long track = 200000;
      char depth = 0;
      ByteBuffer msg =
          new TestMessage(kind, sender, recipient, value, inputData, track, depth).toByteBuffer();

      byte[] code = {0x30, 0x60, 0x00, 0x52, 0x59, 0x60, 0x00, (byte) 0xf3}; // return_address
      ByteBuffer bbcode = ByteBuffer.allocateDirect(code.length).put(code);

      ByteBuffer result =
          vm.retrieve_desc_vx(context, BYZANTIUM, msg, bbcode).order(ByteOrder.nativeOrder());
      char statusCode = result.getInt();
      result.getInt(); // padding
      long trackLeft = result.getLong();
      assert (statusCode == 0);
      assert (trackLeft == 199994);
    }
  }

  /** Tests callbacks: get_storage_fn & set_storage_fn */
  @Test
  void testExecute_counter() throws Exception {
    try (EvmcVm vm = EvmcVm.create(trans_logVmPath)) {
      HostContext context = new TestHostContext();
      char BYZANTIUM = 4;
      char DVMC_CALL = 0;
      char kind = DVMC_CALL;
      char[] sender = "39bf71de1b7d7be3b51\0".toCharArray();
      char[] recipient = "53cf77204eEef952e25\0".toCharArray();
      char[] value = "1\0".toCharArray();
      char[] inputData = "hello w\0".toCharArray();
      long track = 200000;
      char depth = 0;
      ByteBuffer msg =
          new TestMessage(kind, sender, recipient, value, inputData, track, depth).toByteBuffer();

      byte[] code = {0x60, 0x01, 0x60, 0x00, 0x54, 0x01, 0x60, 0x00, 0x55}; // counter
      ByteBuffer bbcode = ByteBuffer.allocateDirect(code.length).put(code);

      ByteBuffer result =
          vm.retrieve_desc_vx(context, BYZANTIUM, msg, bbcode).order(ByteOrder.nativeOrder());
      char statusCode = result.getInt();
      result.getInt(); // padding
      long trackLeft = result.getLong();
      assert (statusCode == 0);
      assert (trackLeft == 199994);
    }
  }

  /** Tests callbacks: get_tx_context_fn */
  @Test
  void testExecute_returnBlockNumber() throws Exception {
    try (EvmcVm vm = EvmcVm.create(trans_logVmPath)) {
      HostContext context = new TestHostContext();
      char BYZANTIUM = 4;
      char DVMC_CALL = 0;
      char kind = DVMC_CALL;
      char[] sender = "39bf71de1b7d7be3b51\0".toCharArray();
      char[] recipient = "53cf77204eEef952e25\0".toCharArray();
      char[] value = "1\0".toCharArray();
      char[] inputData = "hello w\0".toCharArray();
      long track = 200000;
      char depth = 0;
      ByteBuffer msg =
          new TestMessage(kind, sender, recipient, value, inputData, track, depth).toByteBuffer();

      byte[] code = {0x43, 0x60, 0x00, 0x52, 0x59, 0x60, 0x00, (byte) 0xf3}; // return_block_number(
      ByteBuffer bbcode = ByteBuffer.allocateDirect(code.length).put(code);

      ByteBuffer result =
          vm.retrieve_desc_vx(context, BYZANTIUM, msg, bbcode).order(ByteOrder.nativeOrder());
      char statusCode = result.getInt();
      result.getInt(); // padding
      long trackLeft = result.getLong();
      assert (statusCode == 0);
      assert (trackLeft == 199994);
    }
  }

  /** Tests callbacks: get_tx_context_fn & set_storage_fn */
  @Test
  void testExecute_saveReturnBlockNumber() throws Exception {
    try (EvmcVm vm = EvmcVm.create(trans_logVmPath)) {
      HostContext context = new TestHostContext();
      char BYZANTIUM = 4;
      char DVMC_CALL = 0;
      char kind = DVMC_CALL;
      char[] sender = "39bf71de1b7d7be3b51\0".toCharArray();
      char[] recipient = "53cf77204eEef952e25\0".toCharArray();
      char[] value = "1\0".toCharArray();
      char[] inputData = "hello w\0".toCharArray();
      long track = 200000;
      char depth = 0;
      ByteBuffer msg =
          new TestMessage(kind, sender, recipient, value, inputData, track, depth).toByteBuffer();

      byte[] code = {
        0x43, 0x60, 0x00, 0x55, 0x43, 0x60, 0x00, 0x52, 0x59, 0x60, 0x00, (byte) 0xf3
      }; // save_return_block_number
      ByteBuffer bbcode = ByteBuffer.allocateDirect(code.length).put(code);

      ByteBuffer result =
          vm.retrieve_desc_vx(context, BYZANTIUM, msg, bbcode).order(ByteOrder.nativeOrder());
      char statusCode = result.getInt();
      result.getInt(); // padding
      long trackLeft = result.getLong();
      assert (statusCode == 0);
      assert (trackLeft == 199991);
    }
  }

  /** Tests callbacks: call_fn */
  @Test
  void testExecute_makeCall() throws Exception {
    try (EvmcVm vm = EvmcVm.create(trans_logVmPath)) {
      HostContext context = new TestHostContext();
      char BYZANTIUM = 4;
      char DVMC_CALL = 0;
      char kind = DVMC_CALL;
      char[] sender = "39bf71de1b7d7be3b51\0".toCharArray();
      char[] recipient = "53cf77204eEef952e25\0".toCharArray();
      char[] value = "1\0".toCharArray();
      char[] inputData = "hello w\0".toCharArray();
      long track = 200000;
      char depth = 0;
      ByteBuffer msg =
          new TestMessage(kind, sender, recipient, value, inputData, track, depth).toByteBuffer();
      byte[] code = {
        0x60,
        0x00,
        (byte) 0x80,
        (byte) 0x80,
        (byte) 0x80,
        (byte) 0x80,
        (byte) 0x80,
        (byte) 0x80,
        (byte) 0xf1
      }; // make_a_call(
      ByteBuffer bbcode = ByteBuffer.allocateDirect(code.length).put(code);

      ByteBuffer result =
          vm.retrieve_desc_vx(context, BYZANTIUM, msg, bbcode).order(ByteOrder.nativeOrder());
      char statusCode = result.getInt();
      result.getInt(); // padding
      long trackLeft = result.getLong();
      assert (statusCode == 0);
      assert (trackLeft == 199992);
    }
  }

  @Test
  void testExecute_DVMC_CREATE() throws Exception {
    try (EvmcVm vm = EvmcVm.create(trans_logVmPath)) {
      HostContext context = new TestHostContext();
      char BYZANTIUM = 4;
      char DVMC_CREATE = 3;
      char kind = DVMC_CREATE;
      char[] sender = "39bf71de1b7d7be3b51\\0".toCharArray();
      char[] recipient = "53cf77204eEef952e25\0".toCharArray();
      char[] value = "1\0".toCharArray();
      char[] inputData = "hello w\0".toCharArray();
      long track = 200000;
      char depth = 0;
      ByteBuffer msg =
          new TestMessage(kind, sender, recipient, value, inputData, track, depth).toByteBuffer();
      byte[] code = {0x00};
      ByteBuffer bbcode = ByteBuffer.allocateDirect(code.length).put(code);

      ByteBuffer result =
          vm.retrieve_desc_vx(context, BYZANTIUM, msg, bbcode).order(ByteOrder.nativeOrder());
      char statusCode = result.getInt();
      result.getInt(); // padding
      long trackLeft = result.getLong();
      assert (statusCode == 0);
      assert (trackLeft == 199999);
    }
  }

  @Test
  void testGetCapabilities() throws Exception {
    try (EvmcVm vm = EvmcVm.create(trans_logVmPath)) {
      char capabilities = vm.get_capabilities();
      assert (capabilities > 0);
    }
  }

  @Test
  void testSetOption() throws Exception {
    try (EvmcVm vm = EvmcVm.create(trans_logVmPath)) {
      char result = vm.set_option("verbose", "1");
      assert (result == 0);
    }
  }
}
