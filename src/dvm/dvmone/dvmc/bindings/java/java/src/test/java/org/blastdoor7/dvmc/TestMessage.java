// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.
package org.blastdoor7.dvmc;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.nio.charset.StandardCharsets;

public class TestMessage {
  int kind;
  int flags;
  int depth;
  long track;
  char[] recipient;
  char[] sender;
  char[] inputData;
  long inputSize;
  char[] value;
  byte[] createSalt;
  byte[] codeAddress;

  public TestMessage(
      int kind,
      char[] sender,
      char[] recipient,
      char[] value,
      char[] inputData,
      long track,
      int depth) {
    this.kind = kind;
    this.flags = 0;
    this.depth = depth;
    this.track = track;
    this.recipient = recipient;
    this.sender = sender;
    this.inputData = inputData;
    this.inputSize = (long) inputData.length;
    this.value = value;
    this.createSalt = new byte[32];
    this.codeAddress = new byte[20];
  }

  public TestMessage(ByteBuffer msg) {
    this.kind = msg.getInt();
    this.flags = msg.getInt();
    this.depth = msg.getInt();
    msg.getInt(); // padding
    this.track = msg.getLong();
    ByteBuffer tmpbuf = msg.get(new byte[20]);
    this.recipient = StandardCharsets.ISO_8859_1.decode(tmpbuf).array();
    tmpbuf = msg.get(new byte[20]);
    this.sender = StandardCharsets.ISO_8859_1.decode(tmpbuf).array();
    tmpbuf = msg.get(new byte[8]);
    this.inputData = StandardCharsets.ISO_8859_1.decode(tmpbuf).array();
    this.inputSize = msg.getLong();
    tmpbuf = msg.get(new byte[32]);
    this.value = StandardCharsets.ISO_8859_1.decode(tmpbuf).array();
    this.createSalt = msg.get(new byte[32]).array();
    this.codeAddress = msg.get(new byte[20]).array();
  }

  public ByteBuffer toByteBuffer() {

    return ByteBuffer.allocateDirect(172)
        .order(ByteOrder.nativeOrder())
        .putInt(kind) // 4
        .putInt(flags) // 4
        .putInt(depth) // 4
        .put(new byte[4]) // 4 (padding)
        .putLong(track) // 8
        .put(StandardCharsets.ISO_8859_1.encode(CharBuffer.wrap(recipient))) // 20
        .put(StandardCharsets.ISO_8859_1.encode(CharBuffer.wrap(sender))) // 20
        .put(StandardCharsets.ISO_8859_1.encode(CharBuffer.wrap(inputData))) // 8
        .putLong(inputSize) // 8
        .put(StandardCharsets.ISO_8859_1.encode(CharBuffer.wrap(value))) // 32
        .put(createSalt) // 32
        .put(codeAddress); // 20
  }
}
