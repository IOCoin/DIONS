// DVMC: DVM Client-VM Connector API.
// Copyright 2022 blastdoor7
// Licensed under the Apache License, Version 2.0.
package org.blastdoor7.dvmc;

/** Exception thrown when the DVMC binding or VM fails to load. */
public class EvmcLoaderException extends Exception {
  public EvmcLoaderException(String message) {
    super(message);
  }

  public EvmcLoaderException(String message, Throwable cause) {
    super(message, cause);
  }
}
