// Code generated by Wire protocol buffer compiler, do not edit.
// Source file: GoogleMapsDirection.proto at 1:1
package me.brosa.mapsup;

import com.squareup.wire.FieldEncoding;
import com.squareup.wire.Message;
import com.squareup.wire.ProtoAdapter;
import com.squareup.wire.ProtoReader;
import com.squareup.wire.ProtoWriter;
import com.squareup.wire.WireEnum;
import com.squareup.wire.WireField;
import com.squareup.wire.internal.Internal;
import java.io.IOException;
import java.lang.Object;
import java.lang.Override;
import java.lang.String;
import java.lang.StringBuilder;
import okio.ByteString;

public final class GoogleMapsDirection extends Message<GoogleMapsDirection, GoogleMapsDirection.Builder> {
  public static final ProtoAdapter<GoogleMapsDirection> ADAPTER = new ProtoAdapter_GoogleMapsDirection();

  private static final long serialVersionUID = 0L;

  public static final TurnDirection DEFAULT_TDIRECTION = TurnDirection.Right;

  public static final String DEFAULT_DISTANCE = "";

  @WireField(
      tag = 4,
      adapter = ".GoogleMapsDirection$TurnDirection#ADAPTER",
      label = WireField.Label.REQUIRED
  )
  public final TurnDirection tdirection;

  @WireField(
      tag = 1,
      adapter = "com.squareup.wire.ProtoAdapter#STRING",
      label = WireField.Label.REQUIRED
  )
  public final String distance;

  public GoogleMapsDirection(TurnDirection tdirection, String distance) {
    this(tdirection, distance, ByteString.EMPTY);
  }

  public GoogleMapsDirection(TurnDirection tdirection, String distance, ByteString unknownFields) {
    super(ADAPTER, unknownFields);
    this.tdirection = tdirection;
    this.distance = distance;
  }

  @Override
  public Builder newBuilder() {
    Builder builder = new Builder();
    builder.tdirection = tdirection;
    builder.distance = distance;
    builder.addUnknownFields(unknownFields());
    return builder;
  }

  @Override
  public boolean equals(Object other) {
    if (other == this) return true;
    if (!(other instanceof GoogleMapsDirection)) return false;
    GoogleMapsDirection o = (GoogleMapsDirection) other;
    return unknownFields().equals(o.unknownFields())
        && tdirection.equals(o.tdirection)
        && distance.equals(o.distance);
  }

  @Override
  public int hashCode() {
    int result = super.hashCode;
    if (result == 0) {
      result = unknownFields().hashCode();
      result = result * 37 + tdirection.hashCode();
      result = result * 37 + distance.hashCode();
      super.hashCode = result;
    }
    return result;
  }

  @Override
  public String toString() {
    StringBuilder builder = new StringBuilder();
    builder.append(", tdirection=").append(tdirection);
    builder.append(", distance=").append(distance);
    return builder.replace(0, 2, "GoogleMapsDirection{").append('}').toString();
  }

  public static final class Builder extends Message.Builder<GoogleMapsDirection, Builder> {
    public TurnDirection tdirection;

    public String distance;

    public Builder() {
    }

    public Builder tdirection(TurnDirection tdirection) {
      this.tdirection = tdirection;
      return this;
    }

    public Builder distance(String distance) {
      this.distance = distance;
      return this;
    }

    @Override
    public GoogleMapsDirection build() {
      if (tdirection == null
          || distance == null) {
        throw Internal.missingRequiredFields(tdirection, "tdirection",
            distance, "distance");
      }
      return new GoogleMapsDirection(tdirection, distance, super.buildUnknownFields());
    }
  }

  public enum TurnDirection implements WireEnum {
    Right(0),

    Left(1),

    Straight(2),

    TurnAround(3);

    public static final ProtoAdapter<TurnDirection> ADAPTER = ProtoAdapter.newEnumAdapter(TurnDirection.class);

    private final int value;

    TurnDirection(int value) {
      this.value = value;
    }

    /**
     * Return the constant for {@code value} or null.
     */
    public static TurnDirection fromValue(int value) {
      switch (value) {
        case 0: return Right;
        case 1: return Left;
        case 2: return Straight;
        case 3: return TurnAround;
        default: return null;
      }
    }

    @Override
    public int getValue() {
      return value;
    }
  }

  private static final class ProtoAdapter_GoogleMapsDirection extends ProtoAdapter<GoogleMapsDirection> {
    ProtoAdapter_GoogleMapsDirection() {
      super(FieldEncoding.LENGTH_DELIMITED, GoogleMapsDirection.class);
    }

    @Override
    public int encodedSize(GoogleMapsDirection value) {
      return TurnDirection.ADAPTER.encodedSizeWithTag(4, value.tdirection)
          + ProtoAdapter.STRING.encodedSizeWithTag(1, value.distance)
          + value.unknownFields().size();
    }

    @Override
    public void encode(ProtoWriter writer, GoogleMapsDirection value) throws IOException {
      TurnDirection.ADAPTER.encodeWithTag(writer, 4, value.tdirection);
      ProtoAdapter.STRING.encodeWithTag(writer, 1, value.distance);
      writer.writeBytes(value.unknownFields());
    }



    @Override
    public GoogleMapsDirection decode(ProtoReader reader) throws IOException {
      Builder builder = new Builder();
      long token = reader.beginMessage();
      for (int tag; (tag = reader.nextTag()) != -1;) {
        switch (tag) {
          case 4: {
            try {
              builder.tdirection(TurnDirection.ADAPTER.decode(reader));
            } catch (ProtoAdapter.EnumConstantNotFoundException e) {
              builder.addUnknownField(tag, FieldEncoding.VARINT, (long) e.value);
            }
            break;
          }
          case 1: builder.distance(ProtoAdapter.STRING.decode(reader)); break;
          default: {
            FieldEncoding fieldEncoding = reader.peekFieldEncoding();
            Object value = fieldEncoding.rawProtoAdapter().decode(reader);
            builder.addUnknownField(tag, fieldEncoding, value);
          }
        }
      }
      reader.endMessage(token);
      return builder.build();
    }

    @Override
    public GoogleMapsDirection redact(GoogleMapsDirection value) {
      Builder builder = value.newBuilder();
      builder.clearUnknownFields();
      return builder.build();
    }
  }
}
