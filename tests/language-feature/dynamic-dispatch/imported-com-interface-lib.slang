//TEST_IGNORE_FILE:

// Library half of `imported-com-interface.slang`.
//
// The COM interface lives in a module of its own, so the main module lowers a
// reference to an interface it does not own and takes the borrowing path in
// `tryBorrowInterfaceFromOwningModule`.

module "imported-com-interface-lib";

[COM("00000000-0000-0000-0000-0000000000AB")]
public interface IComService
{
    float query(float x);
}
