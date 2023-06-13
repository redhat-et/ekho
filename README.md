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

- [Offload mechanisms](docs/offload-mechanisms.org "Offload mechanisms")
