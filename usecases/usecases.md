# Use cases

## Assumptions
- A separate OS will be installed/running on the xPU.

## OPI Networking Use cases

This sections lists the currently supported networking use cases in the OPI.

### Network Services Offload

The Network Services Offload use case will build a foundation for the examples that can be put together for the OPI.  The basic topology is shown in the diagram.  It consists of two servers, with D/IPUs that are connected through a network switch.

![Network Services Offload Use Case](./images/API-Network-Use-Case.png)

## Red Hat Networking Use cases

This sections lists the networking use cases that interest Red Hat for the OPI.

### Kubernetes Networking Infrastructure offload (Vanilla)
Offloading K8s networking infrastructure from Host CPUs to the xPU as is (in order
to save Host CPU for other processing). Freeing up valuable resources for host workloads.

### Kubernetes Networking Infrastructure offload (+ ASIC/FPGA)
Offloading K8s networking infrastructure from Host CPUs to the xPU but also
offloading virtual switching, routing, load balancing, IPSec or NAT to ASIC/FPGA.
Freeing up valuable resources for host workloads.

### Accelerating workloads with RHEL
Offloading virtual switching, routing, load balancing, NAT, IPSec, QoS... to xPUs. Freeing up valuable resources for host workloads.
