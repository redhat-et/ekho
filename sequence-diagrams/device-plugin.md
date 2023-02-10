# xPU Device Plugin (DP)

The xPU DP (maybe just SR-IOV DP), provisions and manages a resource pool
of VFs that can be requested by the Pod.

## Assumptions

- VFs/SFs are uniquely identified by their PCI address. AKA, PCI address is the same on the Host Server as on the xPU.
- DP interacts with the OPI to provision the appropriate VF and not the CNI.

## Opens

- Is this even an option we want to consider? rather than just having a CNI-SHIM manage it all.
- Is the VF really the primary/only interface for the Pod? How would this work with something like prometheus today (if it's gathering app metrics). Won't another interface be needed? or is prometheus moving to the DPU?

## Provisioning a VF/SF to a Pod Options

### DP interacts with OPI xPU Agent via CRDs

![alternative text](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/redhat-et/opi-k8s-networking/main/sequence-diagrams/puml/device-plugin-CRs.puml?token=GHSAT0AAAAAAB5HTHR3WESMKFJPOPC7YV5SY7GKS6Q)

### DP interacts with OPI Host Agent directly

This is not really the K8s way of doing things.

The following diagram shows the host view only, xPU view to be created separately or appended to this diagram.

![alternative text](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/redhat-et/opi-k8s-networking/main/sequence-diagrams/puml/device-plugin.puml?token=GHSAT0AAAAAAB5HTHR2JBP22BUHARERXOOWY7GLBHQ)
