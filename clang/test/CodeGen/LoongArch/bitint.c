// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py UTC_ARGS: --version 5
// RUN: %clang_cc1 -triple loongarch32 -fexperimental-max-bitint-width=1024 -emit-llvm %s -o - | FileCheck %s --check-prefix=LA32
// RUN: %clang_cc1 -triple loongarch64 -fexperimental-max-bitint-width=1024 -emit-llvm %s -o - | FileCheck %s --check-prefix=LA64

/// Testing the alignment and size of _BitInt(N) for different values of N.

void pass_BitInt7(_BitInt(7));
void pass_BitInt65(_BitInt(65));
void pass_BitInt129(_BitInt(129));

// LA32-LABEL: define dso_local void @example_BitInt(
// LA32-SAME: ) #[[ATTR0:[0-9]+]] {
// LA32-NEXT:  [[ENTRY:.*:]]
// LA32-NEXT:    [[L7:%.*]] = alloca i8, align 1
// LA32-NEXT:    [[L65:%.*]] = alloca i128, align 16
// LA32-NEXT:    [[L129:%.*]] = alloca [32 x i8], align 16
// LA32-NEXT:    [[BYVAL_TEMP:%.*]] = alloca i128, align 16
// LA32-NEXT:    [[BYVAL_TEMP3:%.*]] = alloca [32 x i8], align 16
// LA32-NEXT:    store i8 0, ptr [[L7]], align 1
// LA32-NEXT:    store i128 0, ptr [[L65]], align 16
// LA32-NEXT:    store i256 0, ptr [[L129]], align 16
// LA32-NEXT:    [[TMP0:%.*]] = load i8, ptr [[L7]], align 1
// LA32-NEXT:    [[LOADEDV:%.*]] = trunc i8 [[TMP0]] to i7
// LA32-NEXT:    call void @pass_BitInt7(i7 noundef signext [[LOADEDV]])
// LA32-NEXT:    [[TMP1:%.*]] = load i128, ptr [[L65]], align 16
// LA32-NEXT:    [[LOADEDV1:%.*]] = trunc i128 [[TMP1]] to i65
// LA32-NEXT:    [[STOREDV:%.*]] = sext i65 [[LOADEDV1]] to i128
// LA32-NEXT:    store i128 [[STOREDV]], ptr [[BYVAL_TEMP]], align 16
// LA32-NEXT:    call void @pass_BitInt65(ptr dead_on_return noundef [[BYVAL_TEMP]])
// LA32-NEXT:    [[TMP2:%.*]] = load i256, ptr [[L129]], align 16
// LA32-NEXT:    [[LOADEDV2:%.*]] = trunc i256 [[TMP2]] to i129
// LA32-NEXT:    [[STOREDV4:%.*]] = sext i129 [[LOADEDV2]] to i256
// LA32-NEXT:    store i256 [[STOREDV4]], ptr [[BYVAL_TEMP3]], align 16
// LA32-NEXT:    call void @pass_BitInt129(ptr dead_on_return noundef [[BYVAL_TEMP3]])
// LA32-NEXT:    ret void
//
// LA64-LABEL: define dso_local void @example_BitInt(
// LA64-SAME: ) #[[ATTR0:[0-9]+]] {
// LA64-NEXT:  [[ENTRY:.*:]]
// LA64-NEXT:    [[L7:%.*]] = alloca i8, align 1
// LA64-NEXT:    [[L65:%.*]] = alloca i128, align 16
// LA64-NEXT:    [[L129:%.*]] = alloca i256, align 16
// LA64-NEXT:    [[BYVAL_TEMP:%.*]] = alloca i256, align 16
// LA64-NEXT:    store i8 0, ptr [[L7]], align 1
// LA64-NEXT:    store i128 0, ptr [[L65]], align 16
// LA64-NEXT:    store i256 0, ptr [[L129]], align 16
// LA64-NEXT:    [[TMP0:%.*]] = load i8, ptr [[L7]], align 1
// LA64-NEXT:    [[LOADEDV:%.*]] = trunc i8 [[TMP0]] to i7
// LA64-NEXT:    call void @pass_BitInt7(i7 noundef signext [[LOADEDV]])
// LA64-NEXT:    [[TMP1:%.*]] = load i128, ptr [[L65]], align 16
// LA64-NEXT:    [[LOADEDV1:%.*]] = trunc i128 [[TMP1]] to i65
// LA64-NEXT:    call void @pass_BitInt65(i65 noundef [[LOADEDV1]])
// LA64-NEXT:    [[TMP2:%.*]] = load i256, ptr [[L129]], align 16
// LA64-NEXT:    [[LOADEDV2:%.*]] = trunc i256 [[TMP2]] to i129
// LA64-NEXT:    [[STOREDV:%.*]] = sext i129 [[LOADEDV2]] to i256
// LA64-NEXT:    store i256 [[STOREDV]], ptr [[BYVAL_TEMP]], align 16
// LA64-NEXT:    call void @pass_BitInt129(ptr dead_on_return noundef [[BYVAL_TEMP]])
// LA64-NEXT:    ret void
//
void example_BitInt(void) {
    _BitInt(7) l7 = {0};
    _BitInt(65) l65 = {0};
    _BitInt(129) l129 = {0};
    pass_BitInt7(l7);
    pass_BitInt65(l65);
    pass_BitInt129(l129);
}

// LA32-LABEL: define dso_local signext i7 @return_large_BitInt7(
// LA32-SAME: ) #[[ATTR0]] {
// LA32-NEXT:  [[ENTRY:.*:]]
// LA32-NEXT:    ret i7 0
//
// LA64-LABEL: define dso_local signext i7 @return_large_BitInt7(
// LA64-SAME: ) #[[ATTR0]] {
// LA64-NEXT:  [[ENTRY:.*:]]
// LA64-NEXT:    ret i7 0
//
_BitInt(7) return_large_BitInt7(void) { return 0; }
// LA32-LABEL: define dso_local void @return_large_BitInt65(
// LA32-SAME: ptr dead_on_unwind noalias writable sret(i128) align 16 [[AGG_RESULT:%.*]]) #[[ATTR0]] {
// LA32-NEXT:  [[ENTRY:.*:]]
// LA32-NEXT:    [[RESULT_PTR:%.*]] = alloca ptr, align 4
// LA32-NEXT:    store ptr [[AGG_RESULT]], ptr [[RESULT_PTR]], align 4
// LA32-NEXT:    store i128 0, ptr [[AGG_RESULT]], align 16
// LA32-NEXT:    [[TMP0:%.*]] = load i128, ptr [[AGG_RESULT]], align 16
// LA32-NEXT:    [[LOADEDV:%.*]] = trunc i128 [[TMP0]] to i65
// LA32-NEXT:    [[STOREDV:%.*]] = sext i65 [[LOADEDV]] to i128
// LA32-NEXT:    store i128 [[STOREDV]], ptr [[AGG_RESULT]], align 16
// LA32-NEXT:    ret void
//
// LA64-LABEL: define dso_local i65 @return_large_BitInt65(
// LA64-SAME: ) #[[ATTR0]] {
// LA64-NEXT:  [[ENTRY:.*:]]
// LA64-NEXT:    ret i65 0
//
_BitInt(65) return_large_BitInt65(void) { return 0; }
// LA32-LABEL: define dso_local void @return_large_BitInt129(
// LA32-SAME: ptr dead_on_unwind noalias writable sret([32 x i8]) align 16 [[AGG_RESULT:%.*]]) #[[ATTR0]] {
// LA32-NEXT:  [[ENTRY:.*:]]
// LA32-NEXT:    [[RESULT_PTR:%.*]] = alloca ptr, align 4
// LA32-NEXT:    store ptr [[AGG_RESULT]], ptr [[RESULT_PTR]], align 4
// LA32-NEXT:    store i256 0, ptr [[AGG_RESULT]], align 16
// LA32-NEXT:    [[TMP0:%.*]] = load i256, ptr [[AGG_RESULT]], align 16
// LA32-NEXT:    [[LOADEDV:%.*]] = trunc i256 [[TMP0]] to i129
// LA32-NEXT:    [[STOREDV:%.*]] = sext i129 [[LOADEDV]] to i256
// LA32-NEXT:    store i256 [[STOREDV]], ptr [[AGG_RESULT]], align 16
// LA32-NEXT:    ret void
//
// LA64-LABEL: define dso_local void @return_large_BitInt129(
// LA64-SAME: ptr dead_on_unwind noalias writable sret(i256) align 16 [[AGG_RESULT:%.*]]) #[[ATTR0]] {
// LA64-NEXT:  [[ENTRY:.*:]]
// LA64-NEXT:    [[RESULT_PTR:%.*]] = alloca ptr, align 8
// LA64-NEXT:    store ptr [[AGG_RESULT]], ptr [[RESULT_PTR]], align 8
// LA64-NEXT:    store i256 0, ptr [[AGG_RESULT]], align 16
// LA64-NEXT:    [[TMP0:%.*]] = load i256, ptr [[AGG_RESULT]], align 16
// LA64-NEXT:    [[LOADEDV:%.*]] = trunc i256 [[TMP0]] to i129
// LA64-NEXT:    [[STOREDV:%.*]] = sext i129 [[LOADEDV]] to i256
// LA64-NEXT:    store i256 [[STOREDV]], ptr [[AGG_RESULT]], align 16
// LA64-NEXT:    ret void
//
_BitInt(129) return_large_BitInt129(void) { return 0; }
