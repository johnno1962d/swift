//===--- MetadataSource.cpp - Swift Metadata Sources for Reflection -------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2016 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#include "swift/Reflection/MetadataSource.h"

#include <sstream>

using namespace swift;
using namespace reflection;

class PrintMetadataSource
  : public MetadataSourceVisitor<PrintMetadataSource, void> {
  std::ostream &OS;
  unsigned Indent;

  std::ostream &indent(unsigned Amount) {
    for (unsigned i = 0; i < Amount; ++i)
      OS << ' ';
    return OS;
  }

  std::ostream &printHeader(std::string Name) {
    indent(Indent) << '(' << Name;
    return OS;
  }

  template<typename T>
  std::ostream &printField(std::string name, const T &value) {
    if (!name.empty())
      OS << " " << name << "=" << value;
    else
      OS << " " << value;
    return OS;
  }

  void printRec(const MetadataSource *MS) {
    OS << "\n";

    Indent += 2;
    visit(MS);
    Indent -= 2;
  }

  void closeForm() {
    OS << ')';
  }

public:
  PrintMetadataSource(std::ostream &OS, unsigned Indent)
    : OS(OS), Indent(Indent) {}

  void
  visitClosureBindingMetadataSource(const ClosureBindingMetadataSource *CB) {
    printHeader("closure-binding");
    printField("index", CB->getIndex());
    closeForm();
  }

  void
  visitReferenceCaptureMetadataSource(const ReferenceCaptureMetadataSource *RC){
    printHeader("reference-capture");
    printField("index", RC->getIndex());
    closeForm();
  }

  void
  visitMetadataCaptureMetadataSource(const MetadataCaptureMetadataSource *MC){
    printHeader("metadata-capture");
    printField("index", MC->getIndex());
    closeForm();
  }

  void
  visitGenericArgumentMetadataSource(const GenericArgumentMetadataSource *GA) {
    printHeader("generic-argument");
    printField("index", GA->getIndex());
    printRec(GA->getSource());
    closeForm();
  }

  void
  visitParentMetadataSource(const ParentMetadataSource *P) {
    printHeader("parent-of");
    printRec(P->getChild());
    closeForm();
  }

  void
  visitImpossibleMetadataSource(const ImpossibleMetadataSource *I) {
    printHeader("impossible");
    closeForm();
  }
};

class MetadataSourceEncoder
  : public MetadataSourceVisitor<MetadataSourceEncoder> {
  std::stringstream OS;
  bool Finalized  = false;
public:
  void
  visitClosureBindingMetadataSource(const ClosureBindingMetadataSource *CB) {
    OS << 'B';
    OS << CB->getIndex();
  }

  void
  visitReferenceCaptureMetadataSource(const ReferenceCaptureMetadataSource *RC){
    OS << 'R';
    OS << RC->getIndex();
  }

  void
  visitMetadataCaptureMetadataSource(const MetadataCaptureMetadataSource *MC) {
    OS << 'M';
    OS << MC->getIndex();
  }

  void
  visitGenericArgumentMetadataSource(const GenericArgumentMetadataSource *GA) {
    OS << 'G';
    OS << GA->getIndex();
    visit(GA->getSource());
    OS << '_';
  }

  void visitParentMetadataSource(const ParentMetadataSource *P) {
    OS << 'P';
    visit(P->getChild());
    OS << '_';
  }

  void visitImpossibleMetadataSource(const ImpossibleMetadataSource *I) {
    OS << 'I';
  }

  std::string finalize() {
    assert(!Finalized && "Attempted to reuse finalized MetadataSourceEncoder");
    Finalized = true;
    return OS.str();
  }
};

void MetadataSource::dump() const {
  dump(std::cerr, 0);
}

void MetadataSource::dump(std::ostream &OS, unsigned Indent) const {
  PrintMetadataSource(OS, Indent).visit(this);
  OS << std::endl;
}

std::string MetadataSource::encode() const {
  MetadataSourceEncoder Encoder;
  Encoder.visit(this);
  return Encoder.finalize();
}

const ImpossibleMetadataSource *
ImpossibleMetadataSource::Singleton = new ImpossibleMetadataSource();

