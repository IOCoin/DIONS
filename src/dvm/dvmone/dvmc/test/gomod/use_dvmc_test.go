package dvmc_use

import (
	"github.com/blastdoor7/dvmc/v10/bindings/go/dvmc"
	"testing"
)

var trans_logVmPath = "./trans_log-vm.so"

func TestGetVmName(t *testing.T) {
	vm, err := dvmc.Load(trans_logVmPath)
	if err != nil {
		t.Fatalf("%v", err)
	}

	expectedName := "trans_log_vm"
	if name := vm.Name(); name != expectedName {
		t.Errorf("wrong VM name: %s, expected %s", name, expectedName)
	}
}
