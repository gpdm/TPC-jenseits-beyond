{$R+,S+,D-,T-,F-,V+,B-,N-,L+}
{$M 16384,0,0}                { <==== the last 0 is important! }


(**************************************************************)
(*                       CONFRAM.PAS                          *)
(*          configures RAM above the video adapters           *)
(*              to become fully available to DOS.             *)
(*               (c) Ralf Preller 30.8.1988                   *)
(*                                                            *)
(*  Syntax:                                                   *)
(*       CONFRAM  MaxSeg  StartSkipSeg-EndSkipSeg             *)
(*                                                            *)
(*  with MaxSeg        highest memorz segment + 1             *)
(*       StartSkipSeg  first non-available segment address    *)
(*                       with the "video adapter's hole"      *)
(*       EndSkipSeg    first available segment address        *)
(*                       after the "video adapter's hole"     *)
(*                                                            *)
(*  All paramters must be given in hex notation for the       *)
(*  memory segments. Allowed notations are:                   *)
(*                                                            *)
(*  - leading 0 or trailing 'H' are allowed                   *)
(*  - argument sepearator: space, comma or '-'                *)
(*                                                            *)
(*             to be compiled using Turbo Pascal 4.0          *)
(**************************************************************)
PROGRAM ConfRAM;

USES
  Dos;

CONST
  VersionSt     = '1.00';

VAR
  MaxSeg,
  StartSkipSeg,
  EndSkipSeg    : word;

PROCEDURE GetCommandLine;
  CONST
    ETX         = #3;
    HexCharSet  : set of char = ['0'..'9','A'..'F','a'..'f'];
  VAR
    I           : byte;
    ParamSt     : string;

  FUNCTION GetParameter : word;
    VAR
      CharCount : byte;
      Number    : word;
    BEGIN
    CharCount := 0; Number := 0;
    while ParamSt[1] = '0' do Delete (ParamSt, 1, 1);
    while (CharCount < 4) and (ParamSt[1] in HexCharSet)
    do begin
      Number := Number shl 4 + ord(Upcase(ParamSt[1])) - 48
        - 7 * ord(Upcase(ParamSt[1]) >= 'A');
      inc (CharCount);
      Delete (ParamSt, 1, 1);
      end;
    while ParamSt[1] = ' ' do Delete (ParamSt, 1, 1) ;
    GetParameter := Number;
    END;

  BEGIN
  ParamSt := ParamStr(1) + ' ' + ParamStr(2) + ' ' + ParamStr(3)
    + ' ' + ETX;
  repeat
    I := Pos (',', ParamSt);
    if I = 0 then I := Pos ('-', ParamSt);
    if I = 0 then I := Pos ('H ', ParamSt);
    if I = 0 then I := Pos ('h ', ParamSt);
    if I > 0 then ParamSt[I] := ' ';
    until I=0;
  MaxSeg := GetParameter and $FFC0; {only Kbyte steps allowed}
  StartSkipSeg := GetParameter; EndSkipSeg := GetParameter;
  if (ParamSt <> ETX) or (MaxSeg < $4000)
    or (StartSkipSeg >= MaxSeg) or (EndSkipSeg >= MaxSeg)
    or (StartSkipSeg <> 0) and (
    (StartSkipSeg < $4000) or (EndSkipSeg <= StartSkipSeg))
  then begin
    writeln (
      'Arguments Error! Program aborted');
    Halt(1) ;
    end;
  END;

FUNCTION Max (No1, No2 : word) : word;
  BEGIN
  if No1 >= No2 then Max := No1 else Max := No2;
  END;

CONST
  ResetSequence = $1234;

TYPE
  tMCB          = record
                    MCBchar : char;
                    MCBPSP  : word;
                    MCBsize : word;
                    end;

VAR
  I              : word;
  MCBptr         : ^tMCB;
  MemorySize     : word absolute $0:$413;
  Regs           : Registers;
  ResetFlag      : word absolute $0:$472;
  St             : string;

BEGIN
writeln ('CONFRAM  RAM Expansion Version ', VersionSt);
if ParamCount = 0
then writeln ('Currently set RAM size: ', MemorySize,
  ' Kbytes')
else begin
  GetCommandLine ;
  if (MaxSeg shr 6 <> MemorySize)
    and (MaxSeg shr 6 <> succ(MemorySize))  { if disk controller }
  then begin                                {  allocates 1 Kbyte }
    {--- 1st Pass ---}
    MemorySize := MaxSeg shr 6;
    ResetFlag  := ResetSequence;
    Intr ($19, Regs);       { Reboot System }
    end
  else begin
    {--- 2nd Pass ---}
    {--- clear RAM above 640K to avoid parity errors ---}
    for I := Max($A000, EndSkipSeg) to MemorySize shl 6 - 1 do
      Fillchar (ptr(I,0)^, 16, #0);
    if StartSkipSeg > 0 then begin
      for I := $A000 to StartSkipSeg-1 do
        Fillchar (ptr(I,0)^, 16, #0);

      {--- patch Memory Control Blocks ---}
      MCBptr := ptr (PrefixSeg-1, 0);
      MCBptr^.MCBsize := StartSkipSeg - PrefixSeg - 1;
      MCBptr := ptr (StartSkipSeg-1, 0);
      MCBptr^.MCBchar := 'M';
      MCBptr^.MCBPSP := StartSkipSeg;
      MCBptr^.MCBsize := EndSkipSeg - StartSkipSeg;
      MCBptr := ptr (EndSkipSeg, 0);
      MCBptr^.MCBchar := 'Z';
      MCBptr^.MCBPSP := 0;
      MCBptr^.MCBsize := MaxSeg - EndSkipSeg - 1;
      end;
    writeln ( ' RAM available to DOS now ',
      MemorySize - (EndSkipSeg - StartSkipSeg + $3F) shr 6,
      ' Kbytes ' ) ;
    end;
  end;
END.
