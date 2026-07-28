import { Text, View } from "react-native";

interface InfoCardProps {
  label: string;
  value: string;
}

export default function InfoCard({ label, value }: InfoCardProps) {
  return (
    <View className="w-[48%] rounded-2xl border border-slate-200 bg-white p-4">
      <Text className="text-sm font-medium text-slate-500">{label}</Text>

      <Text className="mt-2 text-lg font-bold text-slate-900" numberOfLines={1}>
        {value}
      </Text>
    </View>
  );
}
