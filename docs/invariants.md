A cell never observes hardware it does not own.
No PCIe device is visible to more than one cell.
CPU cores are never time-shared across cells.
SMT siblings are never split between cells.
The dashboard cannot access kernel memory of any cell.
The substrate does not emulate hardware; it allocates it.