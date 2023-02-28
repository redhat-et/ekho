# Host provisioning xPU VF/SF

The main idea behind this document is to specify the flow of VF/SF allocation from the Host POV if the networking infrastructure is provided by a xPU.

## Assumptions

- This is greenfield.

- VFs/SFs are uniquely identified by their PCI address. AKA, PCI address is the same on the Host Server as on the xPU. This is needed to track VFs/SFs as they move through networking namespaces (if_index is not unique).

- At a high level the same scenario will work for both Single Cluster or Multi-cluster.

- This will work for primary or secondary networking.

## Opens

- Is the VF/SF really the primary/only interface for the Pod? How would this work with something like prometheus today (if it's gathering app metrics). Won't another interface be needed? or is prometheus moving to the DPU?

### Provisioning via CRDs

#### Entities and roles

- **xPU Host Agent**
- **Device Plugin**
- **CNI**

The following diagram shows the host view only, xPU view to be created separately or appended to this diagram.

#### Single Cluster

![host-provisioning-xPU-VF](./images/host-provisioning-xPU-VF.png)

#### Multi Cluster

![host-provisioning-xPU-VF-multi-cluster](./images/host-provisioning-xPU-VF-multi-cluster.png)
