# Extending Kernel Hardware Offload

## Introduction

There are a limited number of APIs available to NIC and DPU vendors (and users) to leverage hardware offloading capabilities offered by the kernel. Today these Kernel APIs include: TC, Netfilter, Switchdev and fdb_notifier.

DPU/NIC vendors that wish to enable and/or use hardware offload capabilities need to implement one or more of these API as part of their device driver and then follow a lengthy process to upstream their driver to the Kernel. Vendors (and users) are also limited by what these APIs have to offer. If their hardware supports more capabilities than what the APIs support, they must either find a way to extend the Kernel API (which may or may not be accepted upstream), or find another solution to leverage these capabilities which typically includes maintaining an out of tree driver alongside any additional software to support NIC/DPU management.

The goal of this POC is to explore an alternative method that can work alongside the Kernel APIs to enable users to take full advantage of the Hardware capabilities offered by the NICs or DPUs.

## Purpose of the Proof of Concept (POC)

The purpose of the POC is to explore if hardware offload enablement can be decoupled from kernel driver development. There are multiple mechanisms to evaluate that could each contribute to an overall solution for configuring and using NIC/DPU hardware offloads with minimal driver development.

- Can netlink listeners be used to enable an offload path entirely in user space.
- Can eBPF be used to extend the offload functionality of kernel drivers
- Can eBPF be used to provide a notification path to user space for kernel notifiers.
- Can eBPF be used to enable new types of kernel offload beyond the current scope of what Kernel offload APIs support

If offload processing can be moved outside of the kernel then it would be possible to use vendor libraries from user space to do the hardware programming.

## Overview of the Technical Solution

The following document provides an overview of the offload APIs the kernel uses to talk to
drivers and the ways that BPF could be used to hook into those APIs from user space.

 - [Offload mechanisms](docs/offload-mechanisms.org "Offload mechanisms")

### Mirroring kernel networking state

The primary mechanism we want to use for mirroring the kernel networking state is to listen to
existing netlink notifications to learn the networking state and keep in sync with the
kernel. Netlink will provide an accurate view of the configured state and should provide
notifications about derived state. We expect there to be gaps in what netlink notifications
provide so we will need to consider:

 - Pushing patches upstream to fill the netlink gaps (slow and not guaranteed to be accepted)
 - Using BPF kprobes to provide an additional notification stream

A significant gap when using netlink to mirror state is that stats need to flow in the opposite
direction. Stats are normally collected by the kernel but stats for hardware offloaded flows are
typically collected by drivers on demand when a netlink stats request is received. It will be
necessary to hook in to the request for stats and populate the values using an out-of-band
mechanism. We envisage using a BPF "firmware" program to populate the stats structures from data
collected into BPF maps by a user space process. We need to explore the following approaches:

 - Using BPF struct_ops as a way to attach a BPF program to the FLOW_CLS_STATS command in a
   netdev (implies UAPI changes which is discouraged)
 - Using a combination of BPF kprobes and kfuncs to achieve the same thing (worse UX)

Another known gap with netlink notifications is that the kernel also uses notifiers to inform
drivers about offloadable state. These notifiers include the switchdev notifier which is used to
push MAC and VLAN offloads to drivers and the FIB notifier which is used to push L3 forwarding
rules offloads to drivers. The proof of concept [bpf-notifier](bpf-notifier/) uses BPF kprobes
that attach to the notifiers to send the notifications to user space via BPF ring buffers.

### Extending offloads with BPF "firmware"

The previously discussed offload methods are passive, either listening to netlink notifications
entirely in user space or using kprobes to eavesdrop the notifiers and pass the notifications on
to user space.

A more active approach to extending hardware offload would be to provide explicit hooks where
BPF programs implement the hardware offload APIs on behalf of a driver. The offload mechanism
could be one of:

 - Passing the offload request to user space where it can be fulfilled asynchronously.
 - Using kfuncs to directly implement the offload within the driver (security / sandbox
   limitations are unknown)

This method of using BPF firmware for drivers is attractive because it decouples offload feature
development from the upstream kernel development and release life cycle. It is also an explicit
integration with the kernel offload APIs that gives the kernel visibility of success or failure.

### Enabling new types of offload

TODO

## Development Phases

### Phase 1 – Netlink + Stats offload

In phase 1 we propose to investigate a minimum viable product:

 - Netlink notifications for mirroring networking state to the DPU hardware.
 - Implement BPF struct_ops for the flow offload API and use it to integrate hardware stats
   collection into a driver.

The proof of concept implementation can be done in the netdevsim driver which exists to test
driver APIs without requiring any hardware.

### Phase 2 – Implement offloads with BPF firmware

In phase 2 we propose to investigate using BPF firmware to implement hardware offloads from
within a driver:

 - Extend the struct_ops from phase 1 to add hardware offload operations
 - Investigate using BPF kfuncs to implement the hardware control part of the offload mechanism.

### Phase 3 – Enabling new types of offload

In phase 3 we propose to investigate enabling entirely new types of offload without needing to
upstream new APIs into the kernel.
