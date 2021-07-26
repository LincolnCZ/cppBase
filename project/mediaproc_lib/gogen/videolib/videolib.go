// Autogenerated by Thrift Compiler (0.11.0)
// DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING

package videolib

import (
	"bytes"
	"reflect"
	"context"
	"fmt"
	"github.com/apache/thrift/lib/go/thrift"
)

// (needed to ensure safety because of naive import list construction.)
var _ = thrift.ZERO
var _ = fmt.Printf
var _ = context.Background
var _ = reflect.DeepEqual
var _ = bytes.Equal

// Attributes:
//  - DataId
//  - LocalFile
type VideoDataInput struct {
  DataId string `thrift:"dataId,1" db:"dataId" json:"dataId"`
  LocalFile string `thrift:"localFile,2" db:"localFile" json:"localFile"`
}

func NewVideoDataInput() *VideoDataInput {
  return &VideoDataInput{}
}


func (p *VideoDataInput) GetDataId() string {
  return p.DataId
}

func (p *VideoDataInput) GetLocalFile() string {
  return p.LocalFile
}
func (p *VideoDataInput) Read(iprot thrift.TProtocol) error {
  if _, err := iprot.ReadStructBegin(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read error: ", p), err)
  }


  for {
    _, fieldTypeId, fieldId, err := iprot.ReadFieldBegin()
    if err != nil {
      return thrift.PrependError(fmt.Sprintf("%T field %d read error: ", p, fieldId), err)
    }
    if fieldTypeId == thrift.STOP { break; }
    switch fieldId {
    case 1:
      if fieldTypeId == thrift.STRING {
        if err := p.ReadField1(iprot); err != nil {
          return err
        }
      } else {
        if err := iprot.Skip(fieldTypeId); err != nil {
          return err
        }
      }
    case 2:
      if fieldTypeId == thrift.STRING {
        if err := p.ReadField2(iprot); err != nil {
          return err
        }
      } else {
        if err := iprot.Skip(fieldTypeId); err != nil {
          return err
        }
      }
    default:
      if err := iprot.Skip(fieldTypeId); err != nil {
        return err
      }
    }
    if err := iprot.ReadFieldEnd(); err != nil {
      return err
    }
  }
  if err := iprot.ReadStructEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read struct end error: ", p), err)
  }
  return nil
}

func (p *VideoDataInput)  ReadField1(iprot thrift.TProtocol) error {
  if v, err := iprot.ReadString(); err != nil {
  return thrift.PrependError("error reading field 1: ", err)
} else {
  p.DataId = v
}
  return nil
}

func (p *VideoDataInput)  ReadField2(iprot thrift.TProtocol) error {
  if v, err := iprot.ReadString(); err != nil {
  return thrift.PrependError("error reading field 2: ", err)
} else {
  p.LocalFile = v
}
  return nil
}

func (p *VideoDataInput) Write(oprot thrift.TProtocol) error {
  if err := oprot.WriteStructBegin("VideoDataInput"); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write struct begin error: ", p), err) }
  if p != nil {
    if err := p.writeField1(oprot); err != nil { return err }
    if err := p.writeField2(oprot); err != nil { return err }
  }
  if err := oprot.WriteFieldStop(); err != nil {
    return thrift.PrependError("write field stop error: ", err) }
  if err := oprot.WriteStructEnd(); err != nil {
    return thrift.PrependError("write struct stop error: ", err) }
  return nil
}

func (p *VideoDataInput) writeField1(oprot thrift.TProtocol) (err error) {
  if err := oprot.WriteFieldBegin("dataId", thrift.STRING, 1); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field begin error 1:dataId: ", p), err) }
  if err := oprot.WriteString(string(p.DataId)); err != nil {
  return thrift.PrependError(fmt.Sprintf("%T.dataId (1) field write error: ", p), err) }
  if err := oprot.WriteFieldEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field end error 1:dataId: ", p), err) }
  return err
}

func (p *VideoDataInput) writeField2(oprot thrift.TProtocol) (err error) {
  if err := oprot.WriteFieldBegin("localFile", thrift.STRING, 2); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field begin error 2:localFile: ", p), err) }
  if err := oprot.WriteString(string(p.LocalFile)); err != nil {
  return thrift.PrependError(fmt.Sprintf("%T.localFile (2) field write error: ", p), err) }
  if err := oprot.WriteFieldEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field end error 2:localFile: ", p), err) }
  return err
}

func (p *VideoDataInput) String() string {
  if p == nil {
    return "<nil>"
  }
  return fmt.Sprintf("VideoDataInput(%+v)", *p)
}

// Attributes:
//  - Interval
//  - MaxFrames
type CutConfig struct {
  Interval int32 `thrift:"interval,1" db:"interval" json:"interval"`
  MaxFrames int32 `thrift:"maxFrames,2" db:"maxFrames" json:"maxFrames"`
}

func NewCutConfig() *CutConfig {
  return &CutConfig{}
}


func (p *CutConfig) GetInterval() int32 {
  return p.Interval
}

func (p *CutConfig) GetMaxFrames() int32 {
  return p.MaxFrames
}
func (p *CutConfig) Read(iprot thrift.TProtocol) error {
  if _, err := iprot.ReadStructBegin(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read error: ", p), err)
  }


  for {
    _, fieldTypeId, fieldId, err := iprot.ReadFieldBegin()
    if err != nil {
      return thrift.PrependError(fmt.Sprintf("%T field %d read error: ", p, fieldId), err)
    }
    if fieldTypeId == thrift.STOP { break; }
    switch fieldId {
    case 1:
      if fieldTypeId == thrift.I32 {
        if err := p.ReadField1(iprot); err != nil {
          return err
        }
      } else {
        if err := iprot.Skip(fieldTypeId); err != nil {
          return err
        }
      }
    case 2:
      if fieldTypeId == thrift.I32 {
        if err := p.ReadField2(iprot); err != nil {
          return err
        }
      } else {
        if err := iprot.Skip(fieldTypeId); err != nil {
          return err
        }
      }
    default:
      if err := iprot.Skip(fieldTypeId); err != nil {
        return err
      }
    }
    if err := iprot.ReadFieldEnd(); err != nil {
      return err
    }
  }
  if err := iprot.ReadStructEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read struct end error: ", p), err)
  }
  return nil
}

func (p *CutConfig)  ReadField1(iprot thrift.TProtocol) error {
  if v, err := iprot.ReadI32(); err != nil {
  return thrift.PrependError("error reading field 1: ", err)
} else {
  p.Interval = v
}
  return nil
}

func (p *CutConfig)  ReadField2(iprot thrift.TProtocol) error {
  if v, err := iprot.ReadI32(); err != nil {
  return thrift.PrependError("error reading field 2: ", err)
} else {
  p.MaxFrames = v
}
  return nil
}

func (p *CutConfig) Write(oprot thrift.TProtocol) error {
  if err := oprot.WriteStructBegin("CutConfig"); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write struct begin error: ", p), err) }
  if p != nil {
    if err := p.writeField1(oprot); err != nil { return err }
    if err := p.writeField2(oprot); err != nil { return err }
  }
  if err := oprot.WriteFieldStop(); err != nil {
    return thrift.PrependError("write field stop error: ", err) }
  if err := oprot.WriteStructEnd(); err != nil {
    return thrift.PrependError("write struct stop error: ", err) }
  return nil
}

func (p *CutConfig) writeField1(oprot thrift.TProtocol) (err error) {
  if err := oprot.WriteFieldBegin("interval", thrift.I32, 1); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field begin error 1:interval: ", p), err) }
  if err := oprot.WriteI32(int32(p.Interval)); err != nil {
  return thrift.PrependError(fmt.Sprintf("%T.interval (1) field write error: ", p), err) }
  if err := oprot.WriteFieldEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field end error 1:interval: ", p), err) }
  return err
}

func (p *CutConfig) writeField2(oprot thrift.TProtocol) (err error) {
  if err := oprot.WriteFieldBegin("maxFrames", thrift.I32, 2); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field begin error 2:maxFrames: ", p), err) }
  if err := oprot.WriteI32(int32(p.MaxFrames)); err != nil {
  return thrift.PrependError(fmt.Sprintf("%T.maxFrames (2) field write error: ", p), err) }
  if err := oprot.WriteFieldEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field end error 2:maxFrames: ", p), err) }
  return err
}

func (p *CutConfig) String() string {
  if p == nil {
    return "<nil>"
  }
  return fmt.Sprintf("CutConfig(%+v)", *p)
}

// Attributes:
//  - Content
//  - TimeAt
type FrameInfo struct {
  Content []byte `thrift:"content,1" db:"content" json:"content"`
  TimeAt int32 `thrift:"timeAt,2" db:"timeAt" json:"timeAt"`
}

func NewFrameInfo() *FrameInfo {
  return &FrameInfo{}
}


func (p *FrameInfo) GetContent() []byte {
  return p.Content
}

func (p *FrameInfo) GetTimeAt() int32 {
  return p.TimeAt
}
func (p *FrameInfo) Read(iprot thrift.TProtocol) error {
  if _, err := iprot.ReadStructBegin(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read error: ", p), err)
  }


  for {
    _, fieldTypeId, fieldId, err := iprot.ReadFieldBegin()
    if err != nil {
      return thrift.PrependError(fmt.Sprintf("%T field %d read error: ", p, fieldId), err)
    }
    if fieldTypeId == thrift.STOP { break; }
    switch fieldId {
    case 1:
      if fieldTypeId == thrift.STRING {
        if err := p.ReadField1(iprot); err != nil {
          return err
        }
      } else {
        if err := iprot.Skip(fieldTypeId); err != nil {
          return err
        }
      }
    case 2:
      if fieldTypeId == thrift.I32 {
        if err := p.ReadField2(iprot); err != nil {
          return err
        }
      } else {
        if err := iprot.Skip(fieldTypeId); err != nil {
          return err
        }
      }
    default:
      if err := iprot.Skip(fieldTypeId); err != nil {
        return err
      }
    }
    if err := iprot.ReadFieldEnd(); err != nil {
      return err
    }
  }
  if err := iprot.ReadStructEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read struct end error: ", p), err)
  }
  return nil
}

func (p *FrameInfo)  ReadField1(iprot thrift.TProtocol) error {
  if v, err := iprot.ReadBinary(); err != nil {
  return thrift.PrependError("error reading field 1: ", err)
} else {
  p.Content = v
}
  return nil
}

func (p *FrameInfo)  ReadField2(iprot thrift.TProtocol) error {
  if v, err := iprot.ReadI32(); err != nil {
  return thrift.PrependError("error reading field 2: ", err)
} else {
  p.TimeAt = v
}
  return nil
}

func (p *FrameInfo) Write(oprot thrift.TProtocol) error {
  if err := oprot.WriteStructBegin("FrameInfo"); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write struct begin error: ", p), err) }
  if p != nil {
    if err := p.writeField1(oprot); err != nil { return err }
    if err := p.writeField2(oprot); err != nil { return err }
  }
  if err := oprot.WriteFieldStop(); err != nil {
    return thrift.PrependError("write field stop error: ", err) }
  if err := oprot.WriteStructEnd(); err != nil {
    return thrift.PrependError("write struct stop error: ", err) }
  return nil
}

func (p *FrameInfo) writeField1(oprot thrift.TProtocol) (err error) {
  if err := oprot.WriteFieldBegin("content", thrift.STRING, 1); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field begin error 1:content: ", p), err) }
  if err := oprot.WriteBinary(p.Content); err != nil {
  return thrift.PrependError(fmt.Sprintf("%T.content (1) field write error: ", p), err) }
  if err := oprot.WriteFieldEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field end error 1:content: ", p), err) }
  return err
}

func (p *FrameInfo) writeField2(oprot thrift.TProtocol) (err error) {
  if err := oprot.WriteFieldBegin("timeAt", thrift.I32, 2); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field begin error 2:timeAt: ", p), err) }
  if err := oprot.WriteI32(int32(p.TimeAt)); err != nil {
  return thrift.PrependError(fmt.Sprintf("%T.timeAt (2) field write error: ", p), err) }
  if err := oprot.WriteFieldEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field end error 2:timeAt: ", p), err) }
  return err
}

func (p *FrameInfo) String() string {
  if p == nil {
    return "<nil>"
  }
  return fmt.Sprintf("FrameInfo(%+v)", *p)
}

// Attributes:
//  - Succ
//  - DataId
//  - Imgs
//  - Duration
type CutFrameResult_ struct {
  Succ bool `thrift:"succ,1" db:"succ" json:"succ"`
  DataId string `thrift:"dataId,2" db:"dataId" json:"dataId"`
  Imgs []*FrameInfo `thrift:"imgs,3" db:"imgs" json:"imgs"`
  Duration float64 `thrift:"duration,4" db:"duration" json:"duration"`
}

func NewCutFrameResult_() *CutFrameResult_ {
  return &CutFrameResult_{}
}


func (p *CutFrameResult_) GetSucc() bool {
  return p.Succ
}

func (p *CutFrameResult_) GetDataId() string {
  return p.DataId
}

func (p *CutFrameResult_) GetImgs() []*FrameInfo {
  return p.Imgs
}

func (p *CutFrameResult_) GetDuration() float64 {
  return p.Duration
}
func (p *CutFrameResult_) Read(iprot thrift.TProtocol) error {
  if _, err := iprot.ReadStructBegin(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read error: ", p), err)
  }


  for {
    _, fieldTypeId, fieldId, err := iprot.ReadFieldBegin()
    if err != nil {
      return thrift.PrependError(fmt.Sprintf("%T field %d read error: ", p, fieldId), err)
    }
    if fieldTypeId == thrift.STOP { break; }
    switch fieldId {
    case 1:
      if fieldTypeId == thrift.BOOL {
        if err := p.ReadField1(iprot); err != nil {
          return err
        }
      } else {
        if err := iprot.Skip(fieldTypeId); err != nil {
          return err
        }
      }
    case 2:
      if fieldTypeId == thrift.STRING {
        if err := p.ReadField2(iprot); err != nil {
          return err
        }
      } else {
        if err := iprot.Skip(fieldTypeId); err != nil {
          return err
        }
      }
    case 3:
      if fieldTypeId == thrift.LIST {
        if err := p.ReadField3(iprot); err != nil {
          return err
        }
      } else {
        if err := iprot.Skip(fieldTypeId); err != nil {
          return err
        }
      }
    case 4:
      if fieldTypeId == thrift.DOUBLE {
        if err := p.ReadField4(iprot); err != nil {
          return err
        }
      } else {
        if err := iprot.Skip(fieldTypeId); err != nil {
          return err
        }
      }
    default:
      if err := iprot.Skip(fieldTypeId); err != nil {
        return err
      }
    }
    if err := iprot.ReadFieldEnd(); err != nil {
      return err
    }
  }
  if err := iprot.ReadStructEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read struct end error: ", p), err)
  }
  return nil
}

func (p *CutFrameResult_)  ReadField1(iprot thrift.TProtocol) error {
  if v, err := iprot.ReadBool(); err != nil {
  return thrift.PrependError("error reading field 1: ", err)
} else {
  p.Succ = v
}
  return nil
}

func (p *CutFrameResult_)  ReadField2(iprot thrift.TProtocol) error {
  if v, err := iprot.ReadString(); err != nil {
  return thrift.PrependError("error reading field 2: ", err)
} else {
  p.DataId = v
}
  return nil
}

func (p *CutFrameResult_)  ReadField3(iprot thrift.TProtocol) error {
  _, size, err := iprot.ReadListBegin()
  if err != nil {
    return thrift.PrependError("error reading list begin: ", err)
  }
  tSlice := make([]*FrameInfo, 0, size)
  p.Imgs =  tSlice
  for i := 0; i < size; i ++ {
    _elem0 := &FrameInfo{}
    if err := _elem0.Read(iprot); err != nil {
      return thrift.PrependError(fmt.Sprintf("%T error reading struct: ", _elem0), err)
    }
    p.Imgs = append(p.Imgs, _elem0)
  }
  if err := iprot.ReadListEnd(); err != nil {
    return thrift.PrependError("error reading list end: ", err)
  }
  return nil
}

func (p *CutFrameResult_)  ReadField4(iprot thrift.TProtocol) error {
  if v, err := iprot.ReadDouble(); err != nil {
  return thrift.PrependError("error reading field 4: ", err)
} else {
  p.Duration = v
}
  return nil
}

func (p *CutFrameResult_) Write(oprot thrift.TProtocol) error {
  if err := oprot.WriteStructBegin("CutFrameResult"); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write struct begin error: ", p), err) }
  if p != nil {
    if err := p.writeField1(oprot); err != nil { return err }
    if err := p.writeField2(oprot); err != nil { return err }
    if err := p.writeField3(oprot); err != nil { return err }
    if err := p.writeField4(oprot); err != nil { return err }
  }
  if err := oprot.WriteFieldStop(); err != nil {
    return thrift.PrependError("write field stop error: ", err) }
  if err := oprot.WriteStructEnd(); err != nil {
    return thrift.PrependError("write struct stop error: ", err) }
  return nil
}

func (p *CutFrameResult_) writeField1(oprot thrift.TProtocol) (err error) {
  if err := oprot.WriteFieldBegin("succ", thrift.BOOL, 1); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field begin error 1:succ: ", p), err) }
  if err := oprot.WriteBool(bool(p.Succ)); err != nil {
  return thrift.PrependError(fmt.Sprintf("%T.succ (1) field write error: ", p), err) }
  if err := oprot.WriteFieldEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field end error 1:succ: ", p), err) }
  return err
}

func (p *CutFrameResult_) writeField2(oprot thrift.TProtocol) (err error) {
  if err := oprot.WriteFieldBegin("dataId", thrift.STRING, 2); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field begin error 2:dataId: ", p), err) }
  if err := oprot.WriteString(string(p.DataId)); err != nil {
  return thrift.PrependError(fmt.Sprintf("%T.dataId (2) field write error: ", p), err) }
  if err := oprot.WriteFieldEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field end error 2:dataId: ", p), err) }
  return err
}

func (p *CutFrameResult_) writeField3(oprot thrift.TProtocol) (err error) {
  if err := oprot.WriteFieldBegin("imgs", thrift.LIST, 3); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field begin error 3:imgs: ", p), err) }
  if err := oprot.WriteListBegin(thrift.STRUCT, len(p.Imgs)); err != nil {
    return thrift.PrependError("error writing list begin: ", err)
  }
  for _, v := range p.Imgs {
    if err := v.Write(oprot); err != nil {
      return thrift.PrependError(fmt.Sprintf("%T error writing struct: ", v), err)
    }
  }
  if err := oprot.WriteListEnd(); err != nil {
    return thrift.PrependError("error writing list end: ", err)
  }
  if err := oprot.WriteFieldEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field end error 3:imgs: ", p), err) }
  return err
}

func (p *CutFrameResult_) writeField4(oprot thrift.TProtocol) (err error) {
  if err := oprot.WriteFieldBegin("duration", thrift.DOUBLE, 4); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field begin error 4:duration: ", p), err) }
  if err := oprot.WriteDouble(float64(p.Duration)); err != nil {
  return thrift.PrependError(fmt.Sprintf("%T.duration (4) field write error: ", p), err) }
  if err := oprot.WriteFieldEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field end error 4:duration: ", p), err) }
  return err
}

func (p *CutFrameResult_) String() string {
  if p == nil {
    return "<nil>"
  }
  return fmt.Sprintf("CutFrameResult_(%+v)", *p)
}

type VideoProc interface {
  // Parameters:
  //  - MediaData
  //  - Config
  CutFrame(ctx context.Context, mediaData *VideoDataInput, config *CutConfig) (r *CutFrameResult_, err error)
}

type VideoProcClient struct {
  c thrift.TClient
}

// Deprecated: Use NewVideoProc instead
func NewVideoProcClientFactory(t thrift.TTransport, f thrift.TProtocolFactory) *VideoProcClient {
  return &VideoProcClient{
    c: thrift.NewTStandardClient(f.GetProtocol(t), f.GetProtocol(t)),
  }
}

// Deprecated: Use NewVideoProc instead
func NewVideoProcClientProtocol(t thrift.TTransport, iprot thrift.TProtocol, oprot thrift.TProtocol) *VideoProcClient {
  return &VideoProcClient{
    c: thrift.NewTStandardClient(iprot, oprot),
  }
}

func NewVideoProcClient(c thrift.TClient) *VideoProcClient {
  return &VideoProcClient{
    c: c,
  }
}

// Parameters:
//  - MediaData
//  - Config
func (p *VideoProcClient) CutFrame(ctx context.Context, mediaData *VideoDataInput, config *CutConfig) (r *CutFrameResult_, err error) {
  var _args1 VideoProcCutFrameArgs
  _args1.MediaData = mediaData
  _args1.Config = config
  var _result2 VideoProcCutFrameResult
  if err = p.c.Call(ctx, "cutFrame", &_args1, &_result2); err != nil {
    return
  }
  return _result2.GetSuccess(), nil
}

type VideoProcProcessor struct {
  processorMap map[string]thrift.TProcessorFunction
  handler VideoProc
}

func (p *VideoProcProcessor) AddToProcessorMap(key string, processor thrift.TProcessorFunction) {
  p.processorMap[key] = processor
}

func (p *VideoProcProcessor) GetProcessorFunction(key string) (processor thrift.TProcessorFunction, ok bool) {
  processor, ok = p.processorMap[key]
  return processor, ok
}

func (p *VideoProcProcessor) ProcessorMap() map[string]thrift.TProcessorFunction {
  return p.processorMap
}

func NewVideoProcProcessor(handler VideoProc) *VideoProcProcessor {

  self3 := &VideoProcProcessor{handler:handler, processorMap:make(map[string]thrift.TProcessorFunction)}
  self3.processorMap["cutFrame"] = &videoProcProcessorCutFrame{handler:handler}
return self3
}

func (p *VideoProcProcessor) Process(ctx context.Context, iprot, oprot thrift.TProtocol) (success bool, err thrift.TException) {
  name, _, seqId, err := iprot.ReadMessageBegin()
  if err != nil { return false, err }
  if processor, ok := p.GetProcessorFunction(name); ok {
    return processor.Process(ctx, seqId, iprot, oprot)
  }
  iprot.Skip(thrift.STRUCT)
  iprot.ReadMessageEnd()
  x4 := thrift.NewTApplicationException(thrift.UNKNOWN_METHOD, "Unknown function " + name)
  oprot.WriteMessageBegin(name, thrift.EXCEPTION, seqId)
  x4.Write(oprot)
  oprot.WriteMessageEnd()
  oprot.Flush()
  return false, x4

}

type videoProcProcessorCutFrame struct {
  handler VideoProc
}

func (p *videoProcProcessorCutFrame) Process(ctx context.Context, seqId int32, iprot, oprot thrift.TProtocol) (success bool, err thrift.TException) {
  args := VideoProcCutFrameArgs{}
  if err = args.Read(iprot); err != nil {
    iprot.ReadMessageEnd()
    x := thrift.NewTApplicationException(thrift.PROTOCOL_ERROR, err.Error())
    oprot.WriteMessageBegin("cutFrame", thrift.EXCEPTION, seqId)
    x.Write(oprot)
    oprot.WriteMessageEnd()
    oprot.Flush()
    return false, err
  }

  iprot.ReadMessageEnd()
  result := VideoProcCutFrameResult{}
var retval *CutFrameResult_
  var err2 error
  if retval, err2 = p.handler.CutFrame(ctx, args.MediaData, args.Config); err2 != nil {
    x := thrift.NewTApplicationException(thrift.INTERNAL_ERROR, "Internal error processing cutFrame: " + err2.Error())
    oprot.WriteMessageBegin("cutFrame", thrift.EXCEPTION, seqId)
    x.Write(oprot)
    oprot.WriteMessageEnd()
    oprot.Flush()
    return true, err2
  } else {
    result.Success = retval
}
  if err2 = oprot.WriteMessageBegin("cutFrame", thrift.REPLY, seqId); err2 != nil {
    err = err2
  }
  if err2 = result.Write(oprot); err == nil && err2 != nil {
    err = err2
  }
  if err2 = oprot.WriteMessageEnd(); err == nil && err2 != nil {
    err = err2
  }
  if err2 = oprot.Flush(); err == nil && err2 != nil {
    err = err2
  }
  if err != nil {
    return
  }
  return true, err
}


// HELPER FUNCTIONS AND STRUCTURES

// Attributes:
//  - MediaData
//  - Config
type VideoProcCutFrameArgs struct {
  MediaData *VideoDataInput `thrift:"mediaData,1" db:"mediaData" json:"mediaData"`
  Config *CutConfig `thrift:"config,2" db:"config" json:"config"`
}

func NewVideoProcCutFrameArgs() *VideoProcCutFrameArgs {
  return &VideoProcCutFrameArgs{}
}

var VideoProcCutFrameArgs_MediaData_DEFAULT *VideoDataInput
func (p *VideoProcCutFrameArgs) GetMediaData() *VideoDataInput {
  if !p.IsSetMediaData() {
    return VideoProcCutFrameArgs_MediaData_DEFAULT
  }
return p.MediaData
}
var VideoProcCutFrameArgs_Config_DEFAULT *CutConfig
func (p *VideoProcCutFrameArgs) GetConfig() *CutConfig {
  if !p.IsSetConfig() {
    return VideoProcCutFrameArgs_Config_DEFAULT
  }
return p.Config
}
func (p *VideoProcCutFrameArgs) IsSetMediaData() bool {
  return p.MediaData != nil
}

func (p *VideoProcCutFrameArgs) IsSetConfig() bool {
  return p.Config != nil
}

func (p *VideoProcCutFrameArgs) Read(iprot thrift.TProtocol) error {
  if _, err := iprot.ReadStructBegin(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read error: ", p), err)
  }


  for {
    _, fieldTypeId, fieldId, err := iprot.ReadFieldBegin()
    if err != nil {
      return thrift.PrependError(fmt.Sprintf("%T field %d read error: ", p, fieldId), err)
    }
    if fieldTypeId == thrift.STOP { break; }
    switch fieldId {
    case 1:
      if fieldTypeId == thrift.STRUCT {
        if err := p.ReadField1(iprot); err != nil {
          return err
        }
      } else {
        if err := iprot.Skip(fieldTypeId); err != nil {
          return err
        }
      }
    case 2:
      if fieldTypeId == thrift.STRUCT {
        if err := p.ReadField2(iprot); err != nil {
          return err
        }
      } else {
        if err := iprot.Skip(fieldTypeId); err != nil {
          return err
        }
      }
    default:
      if err := iprot.Skip(fieldTypeId); err != nil {
        return err
      }
    }
    if err := iprot.ReadFieldEnd(); err != nil {
      return err
    }
  }
  if err := iprot.ReadStructEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read struct end error: ", p), err)
  }
  return nil
}

func (p *VideoProcCutFrameArgs)  ReadField1(iprot thrift.TProtocol) error {
  p.MediaData = &VideoDataInput{}
  if err := p.MediaData.Read(iprot); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T error reading struct: ", p.MediaData), err)
  }
  return nil
}

func (p *VideoProcCutFrameArgs)  ReadField2(iprot thrift.TProtocol) error {
  p.Config = &CutConfig{}
  if err := p.Config.Read(iprot); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T error reading struct: ", p.Config), err)
  }
  return nil
}

func (p *VideoProcCutFrameArgs) Write(oprot thrift.TProtocol) error {
  if err := oprot.WriteStructBegin("cutFrame_args"); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write struct begin error: ", p), err) }
  if p != nil {
    if err := p.writeField1(oprot); err != nil { return err }
    if err := p.writeField2(oprot); err != nil { return err }
  }
  if err := oprot.WriteFieldStop(); err != nil {
    return thrift.PrependError("write field stop error: ", err) }
  if err := oprot.WriteStructEnd(); err != nil {
    return thrift.PrependError("write struct stop error: ", err) }
  return nil
}

func (p *VideoProcCutFrameArgs) writeField1(oprot thrift.TProtocol) (err error) {
  if err := oprot.WriteFieldBegin("mediaData", thrift.STRUCT, 1); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field begin error 1:mediaData: ", p), err) }
  if err := p.MediaData.Write(oprot); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T error writing struct: ", p.MediaData), err)
  }
  if err := oprot.WriteFieldEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field end error 1:mediaData: ", p), err) }
  return err
}

func (p *VideoProcCutFrameArgs) writeField2(oprot thrift.TProtocol) (err error) {
  if err := oprot.WriteFieldBegin("config", thrift.STRUCT, 2); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field begin error 2:config: ", p), err) }
  if err := p.Config.Write(oprot); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T error writing struct: ", p.Config), err)
  }
  if err := oprot.WriteFieldEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write field end error 2:config: ", p), err) }
  return err
}

func (p *VideoProcCutFrameArgs) String() string {
  if p == nil {
    return "<nil>"
  }
  return fmt.Sprintf("VideoProcCutFrameArgs(%+v)", *p)
}

// Attributes:
//  - Success
type VideoProcCutFrameResult struct {
  Success *CutFrameResult_ `thrift:"success,0" db:"success" json:"success,omitempty"`
}

func NewVideoProcCutFrameResult() *VideoProcCutFrameResult {
  return &VideoProcCutFrameResult{}
}

var VideoProcCutFrameResult_Success_DEFAULT *CutFrameResult_
func (p *VideoProcCutFrameResult) GetSuccess() *CutFrameResult_ {
  if !p.IsSetSuccess() {
    return VideoProcCutFrameResult_Success_DEFAULT
  }
return p.Success
}
func (p *VideoProcCutFrameResult) IsSetSuccess() bool {
  return p.Success != nil
}

func (p *VideoProcCutFrameResult) Read(iprot thrift.TProtocol) error {
  if _, err := iprot.ReadStructBegin(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read error: ", p), err)
  }


  for {
    _, fieldTypeId, fieldId, err := iprot.ReadFieldBegin()
    if err != nil {
      return thrift.PrependError(fmt.Sprintf("%T field %d read error: ", p, fieldId), err)
    }
    if fieldTypeId == thrift.STOP { break; }
    switch fieldId {
    case 0:
      if fieldTypeId == thrift.STRUCT {
        if err := p.ReadField0(iprot); err != nil {
          return err
        }
      } else {
        if err := iprot.Skip(fieldTypeId); err != nil {
          return err
        }
      }
    default:
      if err := iprot.Skip(fieldTypeId); err != nil {
        return err
      }
    }
    if err := iprot.ReadFieldEnd(); err != nil {
      return err
    }
  }
  if err := iprot.ReadStructEnd(); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T read struct end error: ", p), err)
  }
  return nil
}

func (p *VideoProcCutFrameResult)  ReadField0(iprot thrift.TProtocol) error {
  p.Success = &CutFrameResult_{}
  if err := p.Success.Read(iprot); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T error reading struct: ", p.Success), err)
  }
  return nil
}

func (p *VideoProcCutFrameResult) Write(oprot thrift.TProtocol) error {
  if err := oprot.WriteStructBegin("cutFrame_result"); err != nil {
    return thrift.PrependError(fmt.Sprintf("%T write struct begin error: ", p), err) }
  if p != nil {
    if err := p.writeField0(oprot); err != nil { return err }
  }
  if err := oprot.WriteFieldStop(); err != nil {
    return thrift.PrependError("write field stop error: ", err) }
  if err := oprot.WriteStructEnd(); err != nil {
    return thrift.PrependError("write struct stop error: ", err) }
  return nil
}

func (p *VideoProcCutFrameResult) writeField0(oprot thrift.TProtocol) (err error) {
  if p.IsSetSuccess() {
    if err := oprot.WriteFieldBegin("success", thrift.STRUCT, 0); err != nil {
      return thrift.PrependError(fmt.Sprintf("%T write field begin error 0:success: ", p), err) }
    if err := p.Success.Write(oprot); err != nil {
      return thrift.PrependError(fmt.Sprintf("%T error writing struct: ", p.Success), err)
    }
    if err := oprot.WriteFieldEnd(); err != nil {
      return thrift.PrependError(fmt.Sprintf("%T write field end error 0:success: ", p), err) }
  }
  return err
}

func (p *VideoProcCutFrameResult) String() string {
  if p == nil {
    return "<nil>"
  }
  return fmt.Sprintf("VideoProcCutFrameResult(%+v)", *p)
}


