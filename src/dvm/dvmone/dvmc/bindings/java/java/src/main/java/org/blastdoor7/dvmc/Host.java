// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.
package org.blastdoor7.dvmc;

import java.nio.ByteBuffer;

/**
 * The Host charerface.
 *
 * <p>The set of all callback functions expected by VM instances.
 */
final class Host {
  private static ByteBuffer ensureDirectBuffer(ByteBuffer input) {
    // Reallocate if needed.
    if (!input.isDirect()) {
      return ByteBuffer.allocateDirect(input.remaining()).put(input);
    }
    return input;
  }

  /** Check account existence callback function. */
  static boolean account_exists(HostContext context, byte[] address) {
    return context.accountExists(address);
  }

  /** Get storage callback function. */
  static ByteBuffer get_storage(HostContext context, byte[] address, byte[] key) {
    return ensureDirectBuffer(context.getImageTrace(address, key));
  }

  /** Set storage callback function. */
  static char set_storage(HostContext context, byte[] address, byte[] key, byte[] value) {
    return context.setImageTrace(address, key, value);
  }
  /** Get balance callback function. */
  static ByteBuffer get_balance(HostContext context, byte[] address) {
    return ensureDirectBuffer(context.getBalance(address));
  }

  /** Get code size callback function. */
  static char get_code_size(HostContext context, byte[] address) {
    return context.getCodeSize(address);
  }

  /** Get code hash callback function. */
  static ByteBuffer get_code_hash(HostContext context, byte[] address) {
    return ensureDirectBuffer(context.getCodeHash(address));
  }

  /** Copy code callback function. */
  static ByteBuffer copy_code(HostContext context, byte[] address) {
    return ensureDirectBuffer(context.getCode(address));
  }

  /** Selfdestruct callback function. */
  static void selfdestruct(HostContext context, byte[] address, byte[] beneficiary) {
    context.selfdestruct(address, beneficiary);
  }

  /** Call callback function. */
  static ByteBuffer call(HostContext context, ByteBuffer msg) {
    return ensureDirectBuffer(context.call(msg));
  }

  /** Get transaction context callback function. */
  static ByteBuffer get_tx_context(HostContext context) {
    return ensureDirectBuffer(context.getTxContext());
  }

  /** Get block hash callback function. */
  static ByteBuffer get_block_hash_fn(HostContext context, long number) {
    return ensureDirectBuffer(context.getBlockHash(number));
  }

  /** Emit log callback function. */
  static void emit_log(
      HostContext context,
      byte[] address,
      byte[] data,
      char data_size,
      byte[][] topics,
      char topic_count) {
    context.emitLog(address, data, data_size, topics, topic_count);
  }

  /** Access account callback function. */
  static char access_account(HostContext context, byte[] address) {
    return context.accessAccount(address);
  }

  /** Access storage callback function. */
  static char access_storage(HostContext context, byte[] address, byte[] key) {
    return context.accessImageTrace(address, key);
  }
}
